/*
 * Ffindex
 * written by Andy Hauser <hauser@genzentrum.lmu.de>.
 * Please add your name here if you distribute modified versions.
 * 
 * Ffindex is provided under the Create Commons license "Attribution-ShareAlike
 * 3.0", which basically captures the spirit of the Gnu Public License (GPL).
 * 
 * See:
 * http://creativecommons.org/licenses/by-sa/3.0/
*/

#define _GNU_SOURCE 1
#define _LARGEFILE64_SOURCE 1
#define _FILE_OFFSET_BITS 64

#include <stdlib.h>
#include <stdio.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ffindex.h"
#include "ffutil.h"

#define MAX_FILENAME_LIST_FILES 4096


void usage(char *program_name)
{
    fprintf(stderr, "USAGE: %s [-a|-v] [-s] [-f file]* OUT_DATA_FILE OUT_INDEX_FILE [-d 2ND_DATA_FILE -i 2ND_INDEX_FILE] [DIR_TO_INDEX|FILE]*\n"
                    "\t-a\t\tappend files/indexes, also needed for sorting an already existing ffindex\n"
                    "\t-d FFDATA_FILE\ta second ffindex data file for inserting/appending\n"
                    "\t-i FFINDEX_FILE\ta second ffindex index file for inserting/appending\n"
                    "\t-f FILE\t\tfile containing a list of file names, one per line\n"
                    "\t\t\t-f can be specified up to %d times\n"
                    "\t-s\t\tsort index file, so that the index can queried.\n"
                    "\t\t\tAnother append operations can be done without sorting.\n"
                    "\t-v\t\tprint version and other info then exit\n"
                    "\nEXAMPLES:\n"
                    "\tCreate a new ffindex containing all files from the \"bar/\" directory containing\n"
                    "\tsay myfile1.txt, myfile2.txt and sort (-s) it so that e.g. ffindex_get can use it.\n"
                    "\t\t$ ffindex_build -s foo.ffdata foo.ffindex bar/\n"
                    "\n\tAdd (-a) more files: myfile3.txt, myfile4.txt.\n"
                    "\t\t$ ffindex_build -a foo.ffdata foo.ffindex myfile3.txt myfile4.txt\n"
                    "\n\tOops, forgot to sort it (-s) so do it afterwards:\n"
                    "\t\t$ ffindex_build -as foo.ffdata foo.ffindex\n"
                    "\nDesigned and implemented by Andreas W. Hauser <hauser@genzentrum.lmu.de>.\n",
                    basename(program_name), MAX_FILENAME_LIST_FILES);
}

int main(int argn, char **argv)
{
  int do_append = 0, do_sort = 0, do_unlink = 0, do_version = 0;
  int opt, err = EXIT_SUCCESS;
  char* list_filenames[MAX_FILENAME_LIST_FILES];
  char* list_ffindex_data[MAX_FILENAME_LIST_FILES];
  char* list_ffindex_index[MAX_FILENAME_LIST_FILES];
  size_t list_ffindex_data_index = 0;
  size_t list_ffindex_index_index = 0;
  size_t list_filenames_index = 0;
  while ((opt = getopt(argn, argv, "asuvd:f:i:")) != -1)
  {
    switch (opt)
    {
      case 'a':
        do_append = 1;
        break;
      case 'd':
        if (list_ffindex_data_index == MAX_FILENAME_LIST_FILES) goto error;
        list_ffindex_data[list_ffindex_data_index++] = optarg;
        break;
      case 'i':
        if (list_ffindex_index_index == MAX_FILENAME_LIST_FILES) goto error;
        list_ffindex_index[list_ffindex_index_index++] = optarg;
        break;
      case 'f':
        if (list_filenames_index == MAX_FILENAME_LIST_FILES) goto error;
        list_filenames[list_filenames_index++] = optarg;
        break;
      case 's':
        do_sort = 1;
        break;
      case 'v':
        do_version = 1;
        break;
      default:
error:
        usage(argv[0]);
        return EXIT_FAILURE;
    }
  }

  if(do_version == 1)
  {
    /* Don't you dare running it on a platform where byte != 8 bits */
    printf("%s version %.2f, off_t = %zd bits\n", argv[0], FFINDEX_VERSION, sizeof(off_t) * 8);
    return EXIT_SUCCESS;
  }

  if(argn - optind < 2)
  {
    usage(argv[0]);
    return EXIT_FAILURE;
  }

  if(do_append && do_unlink)
  {
    fprintf(stderr, "ERROR: append (-a) and unlink (-u) are mutually exclusive\n");
    return EXIT_FAILURE;
  }

  if(list_ffindex_data_index != list_ffindex_index_index)
  {
    fprintf(stderr, "ERROR: -d and -i must be specified pairwise\n");
    return EXIT_FAILURE;
  }

  char *data_filename  = argv[optind++];
  char *index_filename = argv[optind++];
  FILE *data_file, *index_file;

  size_t offset = 0;

  /* open index and data file, seek to end if needed */
  if(do_append)
  {
    data_file  = fopen(data_filename, "a");
    if( data_file == NULL) { perror(data_filename); return EXIT_FAILURE; }

    index_file = fopen(index_filename, "a+");
    if(index_file == NULL) { perror(index_filename); err = EXIT_FAILURE; goto close_data; }

    struct stat sb;
    fstat(fileno(data_file), &sb);
    fseek(data_file, sb.st_size, SEEK_SET);
    offset = sb.st_size;

    fstat(fileno(index_file), &sb);
    fseek(index_file, sb.st_size, SEEK_SET);
  }
  else
  {
    struct stat st;

    if(stat(data_filename, &st) == 0) { errno = EEXIST; perror(data_filename); return EXIT_FAILURE; }
    data_file  = fopen(data_filename, "w");
    if( data_file == NULL) { perror(data_filename); return EXIT_FAILURE; }

    if(stat(index_filename, &st) == 0) { errno = EEXIST; perror(index_filename); err = EXIT_FAILURE; goto close_data; }
    index_file = fopen(index_filename, "w+");
    if(index_file == NULL) { perror(index_filename); err = EXIT_FAILURE; goto close_data; }
  }


  /* For each list_file insert */
  if(list_filenames_index > 0)
    for(int i = 0; i < list_filenames_index; i++)
    {
      FILE *list_file = fopen(list_filenames[i], "r");
      if( list_file == NULL) { perror(list_filenames[i]); err = EXIT_FAILURE; goto close_index; }
      if(ffindex_insert_list_file(data_file, index_file, &offset, list_file) < 0)
      {
        perror(list_filenames[i]);
        err = EXIT_FAILURE;
        goto close_index;
      }
    }

  /* Append other ffindexes */
  if(list_ffindex_data_index > 0)
  {
    for(int i = 0; i < list_ffindex_data_index; i++)
    {
      FILE* data_file_to_add  = fopen(list_ffindex_data[i], "r");  if(  data_file_to_add == NULL) { perror(list_ffindex_data[i]); err = EXIT_FAILURE; goto close_index; }
      FILE* index_file_to_add = fopen(list_ffindex_index[i], "r"); if( index_file_to_add == NULL) { perror(list_ffindex_index[i]); err = EXIT_FAILURE; goto close_data_add_file; }
      size_t data_size;
      char *data_to_add = ffindex_mmap_data(data_file_to_add, &data_size);
      if (data_to_add == MAP_FAILED || data_to_add == NULL) { perror(list_ffindex_data[i]); err = EXIT_FAILURE; goto close_index_add_file; }
      ffindex_index_t* index_to_add = ffindex_index_parse(index_file_to_add, 0);
      if (index_to_add == NULL) { perror(list_ffindex_data[i]); err = EXIT_FAILURE; goto munmap_data; }
      for(size_t entry_i = 0; entry_i < index_to_add->n_entries; entry_i++)
      {
        ffindex_entry_t *entry = ffindex_get_entry_by_index(index_to_add, entry_i);
        if (ffindex_insert_memory(data_file, index_file, &offset, ffindex_get_data_by_entry(data_to_add, entry), entry->length - 1, entry->name) == FFINDEX_ERROR) // skip \0 suffix
        {
          perror(list_ffindex_data[i]);
          err = EXIT_FAILURE;
          break;
        }
      }
      ffindex_index_free(index_to_add);
munmap_data:
      ffindex_munmap_data(data_to_add, data_size);
close_index_add_file:
      fclose(index_file_to_add);
close_data_add_file:
      fclose(data_file_to_add);
      if (err == EXIT_FAILURE)
      {
        goto close_index;
      }
    }
  }


  /* Insert files and directories into the index */
  for(int i = optind; i < argn; i++)
  {
    char *path = argv[i];
    struct stat sb;
    if(stat(path, &sb) == -1)
    {
      fferror_print(__FILE__, __LINE__, __func__, path);
      continue;
    }

    if(S_ISDIR(sb.st_mode))
    {
      err = ffindex_insert_dir(data_file, index_file, &offset, path);
      if(err < 0) fferror_print(__FILE__, __LINE__, __func__, path);
    }
    else if(S_ISREG(sb.st_mode))
    {
      err = ffindex_insert_file(data_file, index_file, &offset, path, basename(path));
      if(err < 0) fferror_print(__FILE__, __LINE__, __func__, path);
    }
  }

  /* Sort the index entries and write back */
  if(do_sort)
  {
    rewind(index_file);
    ffindex_index_t* index = ffindex_index_parse(index_file, 0);
    if(index == NULL)
    {
      fferror_print(__FILE__, __LINE__, __func__, index_filename);
      err = EXIT_FAILURE;
      goto close_index;
    }
    fclose(index_file);
    ffindex_sort_index_file(index);
    index_file = fopen(index_filename, "w");
    if(index_file == NULL) { perror(index_filename); err = EXIT_FAILURE; }
    else
    {
      if (ffindex_write(index, index_file) != FFINDEX_OK)
      {
        err = EXIT_FAILURE;
      }
    }
    ffindex_index_free(index);
  }

close_index:
  fclose(index_file);
  if (err == EXIT_FAILURE)
  {
    unlink(index_filename);
  }
close_data:
  fclose(data_file);
  if (err == EXIT_FAILURE)
  {
    unlink(data_filename);
  }
  return err;
}

/* vim: ts=2 sw=2 et
 */
