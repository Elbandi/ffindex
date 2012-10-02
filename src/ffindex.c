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

#include <sys/mman.h> /* mmap, munmap */
#include <sys/stat.h> /* stat, fstat */
#include <sys/types.h> /* DIR */
#include <dirent.h> /* opendir, readdir, closedir */
#include <limits.h>  /* PATH_MAX */
#include <search.h> /* tsearch, tfind, tdelete, twalk */

#include "ext/fmemopen.h" /* For OS not yet implementing this new standard function */
#include "ffutil.h"
#include "ffindex.h"

/* XXX Use page size? */
#define FFINDEX_BUFFER_SIZE 4096
//#define DEBUG

char* ffindex_copyright_text = "Designed and implemented by Andy Hauser <hauser@genzentrum.lmu.de>.";

char* ffindex_copyright()
{
  return ffindex_copyright_text;
}


/* Insert a memory chunk (string even without \0) into ffindex */
int ffindex_insert_memory(FILE *data_file, FILE *index_file, size_t *offset, char *from_start, size_t from_length, char *name)
{
    size_t offset_before = *offset;
    size_t write_size = fwrite(from_start, sizeof(char), from_length, data_file);
    *offset += write_size;
    if(from_length != write_size)
    {
#ifdef DEBUG
      fferror_print(__FILE__, __LINE__, __func__, name);
#endif
      return FFINDEX_ERROR;
    }

    /* Seperate by '\0' and thus also make sure at least one byte is written */
    char buffer[1] = {'\0'};
    if(fwrite(buffer, sizeof(char), 1, data_file) != 1)
    {
#ifdef DEBUG
      fferror_print(__FILE__, __LINE__, __func__, name);
#endif
      return FFINDEX_ERROR;
    }
    *offset += 1;

    /* write index entry */
    if (fprintf(index_file, "%s\t%zd\t%zd\t%d\n", name, offset_before, *offset - offset_before, 0) < 0)
    {
#ifdef DEBUG
        fferror_print(__FILE__, __LINE__, __func__, name);
#endif
        return FFINDEX_ERROR;
    }
    return FFINDEX_OK;
}


/* Insert all file from a list into ffindex */
int ffindex_insert_list_file(FILE *data_file, FILE *index_file, size_t *start_offset, FILE *list_file)
{
  size_t offset = *start_offset;
  char path[PATH_MAX];
  while(fgets(path, PATH_MAX, list_file) != NULL)
    ffindex_insert_file(data_file, index_file, &offset, ffnchomp(path, strlen(path)), basename(path));

  /* update return value */
  *start_offset = offset;
  return FFINDEX_OK;
}


/* Insert all files from directory into ffindex */
int ffindex_insert_dir(FILE *data_file, FILE *index_file, size_t *start_offset, char *input_dir_name)
{
  DIR *dir = opendir(input_dir_name);
  if(dir == NULL)
  {
#ifdef DEBUG
    fferror_print(__FILE__, __LINE__, __func__, input_dir_name);
#endif
    return FFINDEX_ERROR;
  }

  size_t input_dir_name_len = strlen(input_dir_name);
  char path[PATH_MAX];
  strncpy(path, input_dir_name, NAME_MAX);
  if(input_dir_name[input_dir_name_len - 1] != '/')
  {
    path[input_dir_name_len] = '/';
    input_dir_name_len += 1;
  }

  size_t offset = *start_offset;
  struct dirent *entry;
  while((entry = readdir(dir)) != NULL)
  {
    if(entry->d_name[0] == '.')
      continue;
    strncpy(path + input_dir_name_len, entry->d_name, NAME_MAX);
    struct stat sb;
    if(stat(path, &sb) == -1)
    {
#ifdef DEBUG
      fferror_print(__FILE__, __LINE__, __func__, path);
#endif
      continue;
    }
    if(!S_ISREG(sb.st_mode))
      continue;
    if (ffindex_insert_file(data_file, index_file, &offset, path, entry->d_name) != FFINDEX_OK)
    {
      closedir(dir);
      return FFINDEX_ERROR;
    }
  }
  /* update return value */
  *start_offset = offset;

  closedir(dir);
  return FFINDEX_OK;
}


/* Insert one file by path into ffindex */
int ffindex_insert_file(FILE *data_file, FILE *index_file, size_t *offset, const char *path, char *name)
{
    FILE *file = fopen(path, "r");
    if(file == NULL)
      return errno;

    int ret = ffindex_insert_filestream(data_file, index_file, offset, file, name);
    fclose(file);
    return ret;
}

/* Insert one file by handle into ffindex */
int ffindex_insert_filestream(FILE *data_file, FILE *index_file, size_t *offset, FILE* file, char *name)
{
    struct stat sb;
    time_t mtime;
    int fd = fileno(file);
    if (fd != -1 && fstat(fileno(file), &sb) == 0)
    {
        mtime = sb.st_mtime;
    }
    else
    {
        mtime = 0;
    }
    /* copy and paste file to data file */
    char buffer[FFINDEX_BUFFER_SIZE];
    size_t offset_before = *offset;
    size_t read_size;
    while((read_size = fread(buffer, sizeof(char), sizeof(buffer), file)) > 0)
    {
      size_t write_size = fwrite(buffer, sizeof(char), read_size, data_file);
      *offset += write_size;
      if(read_size != write_size)
      {
#ifdef DEBUG
        fferror_print(__FILE__, __LINE__, __func__, name);
#endif
        return FFINDEX_ERROR;
      }
    }
#ifdef DEBUG
    if(ferror(file))
      warn("fread");
#endif

    /* Seperate by '\0' and thus also make sure at least one byte is written */
    buffer[0] = '\0';
    if(fwrite(buffer, sizeof(char), 1, data_file) != 1)
    {
#ifdef DEBUG
      perror("ffindex_insert_filestream");
#endif
      return FFINDEX_ERROR;
    }
    *offset += 1;

    /* write index entry */
    if (fprintf(index_file, "%s\t%zd\t%zd\t%lld\n", name, offset_before, *offset - offset_before, (long long int)mtime) < 0)
    {
#ifdef DEBUG
        fferror_print(__FILE__, __LINE__, __func__, name);
#endif
        return FFINDEX_ERROR;
    }
    return FFINDEX_OK;
}

/* XXX not implemented yet */
int ffindex_restore(FILE *data_file, FILE *index_file, char *input_dir_name)
{
  return FFINDEX_ERROR;
}


char* ffindex_mmap_data(FILE *file, size_t* size)
{
  struct stat sb;
  int fd =  fileno(file);
  if (fd == -1 || fstat(fd, &sb) != 0)
  {
#ifdef DEBUG
    fferror_print(__FILE__, __LINE__, __func__, "fstat failed");
#endif
    return MAP_FAILED;
  }
  *size = sb.st_size;
  return (char*)mmap(NULL, *size, PROT_READ, MAP_PRIVATE, fd, 0);
}

int ffindex_munmap_data(char *data, size_t size)
{
  return munmap(data, size);
}

static int ffindex_compare_entries_by_name(const void *pentry1, const void *pentry2)
{
  ffindex_entry_t* entry1 = (ffindex_entry_t*)pentry1;
  ffindex_entry_t* entry2 = (ffindex_entry_t*)pentry2;
  return strcmp(entry1->name, entry2->name);
}


ffindex_entry_t* ffindex_bsearch_get_entry(ffindex_index_t *index, char *name)
{
  ffindex_entry_t search;
  search.name = name;
  return (ffindex_entry_t*)bsearch(&search, index->entries, index->n_entries, sizeof(ffindex_entry_t), ffindex_compare_entries_by_name);
}


ffindex_index_t* ffindex_index_parse(FILE *index_file, size_t num_start_entries)
{
  if(num_start_entries == 0)
    num_start_entries = 2;
  size_t nbytes = sizeof(ffindex_index_t) + (sizeof(ffindex_entry_t) * num_start_entries);
  ffindex_index_t *index = (ffindex_index_t *)malloc(nbytes);
  if(index == NULL)
  {
#ifdef DEBUG
    fprintf(stderr, "Failed to allocate %ld bytes\n", nbytes);
    fferror_print(__FILE__, __LINE__, __func__, "malloc failed");
#endif
    return NULL;
  }
  index->num_max_entries = num_start_entries;

  index->file = index_file;
  index->index_data = ffindex_mmap_data(index_file, &(index->index_data_size));
  if(index->index_data_size == 0)
    return NULL;
  if(index->index_data == MAP_FAILED)
    return NULL;
  index->type = SORTED_ARRAY; /* XXX Assume a sorted file for now */
  int i = 0;
  char* d = index->index_data;
  char* end;
  /* Faster than scanf per line */
  for(i = 0; d < (index->index_data + index->index_data_size); i++)
  {
    char *p;
    if (i == index->num_max_entries) {
        nbytes = sizeof(ffindex_index_t) + (sizeof(ffindex_entry_t) * index->num_max_entries * 2);
        ffindex_index_t *tmp = (ffindex_index_t *)realloc(index, nbytes);
        if(tmp == NULL)
        {
#ifdef DEBUG
          fprintf(stderr, "Failed to allocate %ld bytes\n", nbytes);
          fferror_print(__FILE__, __LINE__, __func__, "malloc failed");
#endif
          break;
        }
        index = tmp;
        index->num_max_entries *= 2;
    }
    for(p = d; *p != '\t'; p++);
    index->entries[i].name = strndup(d, p - d);
    d = p + 1;
    index->entries[i].offset = strtol(d, &end, 10);
    d = end;
    index->entries[i].length  = strtol(d, &end, 10);
    d = end;
    index->entries[i].mtime  = strtol(d, &end, 10);
    /*
     * eat the remaining chars from line, so current version of library is
     * forward compatible with future versions, if new field is added
     */
    while (*end != '\n') end++;
    d = end + 1; /* +1 for newline */
  }

  index->n_entries = i;

#ifdef DEBUG
  if(index->n_entries == 0)
    warn("index with 0 entries");
#endif

  return index;
}


void ffindex_index_free(ffindex_index_t *index)
{
    for (size_t n = 0; n < index->n_entries; n++) {
        free(index->entries[n].name);
        index->entries[n].name = NULL;
    }
    free(index);
}


ffindex_entry_t* ffindex_get_entry_by_index(ffindex_index_t *index, size_t entry_index)
{
  if(entry_index < index->n_entries)
    return &index->entries[entry_index];
  else
    return NULL;
}

/* Using a function for this looks like overhead. But a more advanced data format,
 * say a compressed one, can do it's magic here. 
 */
char* ffindex_get_data_by_offset(char* data, size_t offset)
{
  return data + offset;
}


char* ffindex_get_data_by_entry(char *data, ffindex_entry_t* entry)
{
  return ffindex_get_data_by_offset(data, entry->offset);
}


char* ffindex_get_data_by_name(char *data, ffindex_index_t *index, char *name)
{
  ffindex_entry_t* entry = ffindex_bsearch_get_entry(index, name);

  if(entry == NULL)
    return NULL;

  return ffindex_get_data_by_entry(data, entry);
}


char* ffindex_get_data_by_index(char *data, ffindex_index_t *index, size_t entry_index)
{
  ffindex_entry_t* entry = ffindex_get_entry_by_index(index, entry_index);

  if(entry == NULL)
    return NULL;

  return ffindex_get_data_by_entry(data, entry);
}


FILE* ffindex_fopen_by_entry(char *data, ffindex_entry_t* entry)
{
  char *filedata = ffindex_get_data_by_offset(data, entry->offset);
  return fmemopen(filedata, entry->length, "r");
}


FILE* ffindex_fopen_by_name(char *data, ffindex_index_t *index, char *filename)
{
  ffindex_entry_t* entry = ffindex_bsearch_get_entry(index, filename);

  if(entry == NULL)
    return NULL;

  return ffindex_fopen_by_entry(data, entry);
}


void ffindex_sort_index_file(ffindex_index_t *index)
{
  qsort(index->entries, index->n_entries, sizeof(ffindex_entry_t), ffindex_compare_entries_by_name);
}


int ffindex_write(ffindex_index_t* index, FILE* index_file)
{
  /* Use tree if available */
  if(index->type == TREE)
    return ffindex_tree_write(index, index_file);

  for(size_t i = 0; i < index->n_entries; i++)
  {
    ffindex_entry_t ffindex_entry = index->entries[i];
    if(fprintf(index_file, "%s\t%zd\t%zd\t%lld\n", ffindex_entry.name, ffindex_entry.offset, ffindex_entry.length, (long long int)ffindex_entry.mtime) < 0)
      return FFINDEX_ERROR;
  }
  return FFINDEX_OK;
}


ffindex_index_t* ffindex_unlink_entries(ffindex_index_t* index, char** sorted_names_to_unlink, int n_names)
{
  int i = index->n_entries - 1;
  /* walk list of names to delete */
  for(int n = n_names - 1; n >= 0;  n--)
  {
    char* name_to_unlink = sorted_names_to_unlink[n];
    /* walk index entries */
    for(; i >= 0; i--)
    {
      int cmp = strcmp(name_to_unlink, index->entries[i].name);
      if(cmp == 0) /* found entry */
      {
        /* Move entries after the unlinked ones to close the gap */
        size_t n_entries_to_move = index->n_entries - i - 1;
        if(n_entries_to_move > 0) /* not last element of array */
          memmove(index->entries + i, index->entries + i + 1, n_entries_to_move * sizeof(ffindex_entry_t));
        index->n_entries--;
        break;
      }
      else if(cmp > 0) /* not found */
        break;
    }
  }

  return index;
}


ffindex_index_t* ffindex_unlink(ffindex_index_t* index, char* name_to_unlink)
{
  /* Use tree if available */
  if(index->type == TREE)
    return ffindex_tree_unlink(index, name_to_unlink);

  ffindex_entry_t* entry = ffindex_bsearch_get_entry(index, name_to_unlink);
  if(entry == NULL)
  {
#ifdef DEBUG
    fprintf(stderr, "Warning: could not find '%s'\n", name_to_unlink);
#endif
    return index;
  }
  /* Move entries after the unlinked one to close the gap */
  size_t n_entries_to_move = index->entries + index->n_entries - entry - 1;
  if(n_entries_to_move > 0) /* not last element of array */
    memmove(entry, entry + 1, n_entries_to_move * sizeof(ffindex_entry_t));
  index->n_entries--;
  return index;
}

/* tree version */

ffindex_entry_t *ffindex_tree_get_entry(ffindex_index_t* index, char* name)
{
  ffindex_entry_t search;
  search.name = name;
  return (ffindex_entry_t *)tfind((const void *)&search, &index->tree_root, ffindex_compare_entries_by_name);
}


ffindex_index_t* ffindex_tree_unlink(ffindex_index_t* index, char* name_to_unlink)
{
  if(index->tree_root == NULL)
  {
#ifdef DEBUG
    fferror_print(__FILE__, __LINE__, __func__, "tree is NULL");
#endif
    return NULL;
  }
  ffindex_entry_t search;
  search.name = name_to_unlink;
  tdelete((const void *)&search, &index->tree_root, ffindex_compare_entries_by_name);
  return index;
}

ffindex_index_t* ffindex_index_as_tree(ffindex_index_t* index)
{
  index->tree_root = NULL;
  for(size_t i = 0; i < index->n_entries; i++)
  {
    ffindex_entry_t *entry = &index->entries[i];
    tsearch((const void *)entry, &index->tree_root, ffindex_compare_entries_by_name);
    //entry = *(ffindex_entry_t **)tsearch((const void *)entry, &index->tree_root, ffindex_compare_entries_by_name);
    //printf("entry find: %s\n", entry->name); 
  }
  index->type = TREE;
  return index;
}


int ffindex_tree_write(ffindex_index_t* index, FILE* index_file)
{
  int ret = FFINDEX_OK;
  void action(const void *node, const VISIT which, const int depth)
  {
    ffindex_entry_t *entry;
    switch (which)
    {
      case preorder:
        break;
      case endorder:
        break;
      case postorder:
      case leaf:
        entry = *(ffindex_entry_t **) node;
        if(fprintf(index_file, "%s\t%zd\t%zd\t%lld\n", entry->name, entry->offset, entry->length, (long long int)entry->mtime) < 0)
          ret = FFINDEX_ERROR;
        break;
    }
  }
  twalk(index->tree_root, action);
  return ret;
}

/* vim: ts=2 sw=2 et
*/
