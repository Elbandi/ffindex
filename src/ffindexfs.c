/*
 * FFindex
 * written by Andy Hauser <hauser@genzentrum.lmu.de>.
 * Please add your name here if you distribute modified versions.
 *.
 * FFindex is provided under the Create Commons license "Attribution-ShareAlike
 * 3.0", which basically captures the spirit of the Gnu Public License (GPL).
 *.
 * See:
 * http://creativecommons.org/licenses/by-sa/3.0/
 *
 * ffindexfs
 * FUSE module to mount ffindex files
*/

// need this to get pwrite().  I have to use setvbuf() instead of
// setlinebuf() later in consequence.
#define _XOPEN_SOURCE 500

#define KEY_HELP (0)
#define KEY_VERSION (1)

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <fuse_opt.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <pthread.h>
#include <sys/mman.h>

#include "ffindex.h"
#include "fflog.h"

static char *rootdir = NULL;

enum FILE_TYPE
{
    ft_real,
    ft_virtual
} filetype;

/**
 * Print usage information
 */
void print_usage(char *program_name) {
    fprintf(stderr, "usage: %s [options] <root-dir> <mountpoint>\n\n", basename(program_name));
    fprintf(stderr,
            "general options:\n"
            "    -o opt,[opt...]        mount options\n"
            "    -h   --help            print help\n"
            "    -v   --version         print version\n"
            "    -f                     don't detach from terminal\n"
            "    -d                     turn on debugging, also implies -f\n"
            "\n");
}

/**
 * Print version information (fuse-zip and FUSE library)
 */
void print_version(char *program_name) {
    /* Don't you dare running it on a platform where byte != 8 bits */
    fprintf(stderr, "%s version %.2f, off_t = %zd bits\n", basename(program_name), FFINDEX_VERSION, sizeof(off_t) * 8);
}

// Report errors to logfile and give -errno to caller
static int ffindex_error(const char *str)
{
    int ret = -errno;

    log_msg("    ERROR %s: %s\n", str, strerror(errno));

    return ret;
}

// Check whether the given user is permitted to perform the given operation on the given

//  All the paths I see are relative to the root of the mounted
//  filesystem.  In order to get to the underlying filesystem, I need to
//  have the mountpoint.  I'll save it away early on in main(), and then
//  whenever I need a path for something I'll call this to construct
//  it.
static int ffindex_fullpath(char fpath[], const char *path, int maxlen)
{
    if (strlen(rootdir) + strlen(path) >= maxlen) {
        errno = ENAMETOOLONG;
        log_msg("Full path too long: '%s%s'", fpath, path);
        return errno;
    }
    strcpy(fpath, rootdir);
    strcat(fpath, path);

    log_msg("    ffindex_fullpath:  rootdir = \"%s\", path = \"%s\", fpath = \"%s\"\n", rootdir, path, fpath);
    return 0;
}

/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int ffindex_getattr(const char *path, struct stat *statbuf)
{
    int retstat = 0;
    char fpath[PATH_MAX + 4];

    log_msg("\nffindex_getattr(path=\"%s\", statbuf=0x%08x)\n", path, statbuf);
    if (ffindex_fullpath(fpath, path, PATH_MAX) != 0) {
        return -ENAMETOOLONG;
    };

    retstat = lstat(fpath, statbuf);
    if (retstat != 0) {
        strcat(fpath, ".idx");
        retstat = lstat(fpath, statbuf);
        if (retstat != 0) { // se letezo fajl, se virtualis fajl
            char *fname = basename((char *)path);
            char *dir = strrchr(fpath, '/');
            if (dir != NULL) {
                strcpy(dir, ".idx");
                int fd = open(fpath, O_RDONLY);
                if (fd < 0) {
                    retstat = -errno;
                } else {
                    ffindex_entry_t* entry = ffindex_get_entry_by_name(fd, fname);
                    if (entry != NULL) {
                        statbuf->st_mode = S_IFREG | 0444;
                        statbuf->st_nlink = 1;
                        statbuf->st_size = entry->length - 1;
                        statbuf->st_blocks = (statbuf->st_size + 511) / 512;
                        statbuf->st_blksize = 4096;
                        statbuf->st_ctime = statbuf->st_mtime = statbuf->st_atime = entry->mtime;
                        retstat = 0;
//                        free(entry->name);
                        free(entry);
                    } else
                        retstat = -ENOENT;

                    close(fd);
                }
            }
        }
        else {
            statbuf->st_mode = S_IFDIR | 0555;
            statbuf->st_nlink = 2;
/*
            statbuf->st_size = 4096;
            statbuf->st_blksize = 4096;
            statbuf->st_blocks = 8;
*/
        }
    }

    log_stat(statbuf);

    return retstat;
}

struct ffindex_dirp {
    enum FILE_TYPE type;
    DIR *dp;
    struct dirent *entry;
    off_t offset;
    int fd;
    ffindex_index_t *index; // TODO: ez ide nemkell, mert hashtablaban van
};

/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int ffindex_opendir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    char fpath[PATH_MAX];
    struct ffindex_dirp *d = malloc(sizeof(struct ffindex_dirp));
    if (d == NULL)
        return -ENOMEM;

    log_msg("\nffindex_opendir(path=\"%s\", fi=0x%08x)\n", path, fi);
    if (ffindex_fullpath(fpath, path, PATH_MAX) != 0) {
        free(d);
        return -ENAMETOOLONG;
    };

    d->type = ft_real;
    d->dp = opendir(fpath);
    if (d->dp == NULL) {
        retstat = -errno;
        if (strlen(fpath) + 4 >= PATH_MAX) {
            errno = ENAMETOOLONG;
            free(d);
            return -errno;
        }
        strcat(fpath, ".idx");
        d->fd = open(fpath, O_RDONLY);
        if (d->fd < 0) {
            free(d);
            return retstat;
        }
        d->index = ffindex_index_parse2(d->fd, 64);
        if (d->index == NULL) {
            close(d->fd);
            free(d);
            return retstat;
        }
        retstat = 0;
        d->type = ft_virtual;
    }

    d->offset = 0;
    d->entry = NULL;

    fi->fh = (unsigned long) d;

    log_fi(fi);

    return retstat;
}

static inline struct ffindex_dirp *get_dirp(struct fuse_file_info *fi)
{
    return (struct ffindex_dirp *) (uintptr_t) fi->fh;
}

/** Read directory
 *
 * This supersedes the old getdir() interface.  New applications
 * should use this.
 *
 * The filesystem may choose between two modes of operation:
 *
 * 1) The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * 2) The readdir implementation keeps track of the offsets of the
 * directory entries.  It uses the offset parameter and always
 * passes non-zero offset to the filler function.  When the buffer
 * is full (or an error happens) the filler function will return
 * '1'.
 *
 * Introduced in version 2.3
 */
int ffindex_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
           struct fuse_file_info *fi)
{
    int retstat = 0;
    struct ffindex_dirp *d = get_dirp(fi);

    log_msg("\nffindex_readdir(path=\"%s\", buf=0x%08x, filler=0x%08x, offset=%lld, fi=0x%08x)\n", path, buf, filler, offset, fi);
    if (d->type == ft_virtual) {
        size_t n;
        switch (offset) {
            case 0:
                if (filler(buf, ".", NULL, ++offset))
                    break;
            case 1:
                if (filler(buf, "..", NULL, ++offset))
                    break;
            default:
                for (n = offset - 2; n < d->index->n_entries; n++) {
                    if (filler(buf, d->index->entries[n].name, NULL, n + 3))
                        break;
            }
        }
    }
    else if (d->type == ft_real) {

        // once again, no need for fullpath -- but note that I need to cast fi->fh
        if (offset != d->offset) {
            seekdir(d->dp, offset);
            d->entry = NULL;
            d->offset = offset;
        }

        while (1) {
            off_t nextoff;

            if (!d->entry) {
                d->entry = readdir(d->dp);
                if (!d->entry)
                    break;
            }

            size_t len = strlen(d->entry->d_name);
            if (len > 4 && strncmp(d->entry->d_name + len - 4, ".dat", 4) == 0) {
                d->entry = NULL;
                d->offset = telldir(d->dp);
                continue;
            }
            if (len > 4 && strncmp(d->entry->d_name + len - 4, ".idx", 4) == 0) {
                d->entry->d_name[len - 4] = '\0';
            };

/*
            memset(&st, 0, sizeof(st));
            st.st_ino = d->entry->d_ino;
            st.st_mode = d->entry->d_type << 12;
*/
            nextoff = telldir(d->dp);
            if (filler(buf, d->entry->d_name, NULL, nextoff))
                break;

            d->entry = NULL;
            d->offset = nextoff;
        }
    }
    else // invalid type
        retstat = EBADF;

    log_fi(fi);

    return retstat;
}

/** Release directory
 *
 * Introduced in version 2.3
 */
int ffindex_releasedir(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    struct ffindex_dirp *d = get_dirp(fi);

    log_msg("\nffindex_releasedir(path=\"%s\", fi=0x%08x)\n", path, fi);
    log_fi(fi);

    if (d->type == ft_virtual) {
        ffindex_index_free(d->index);
        close(d->fd);
    } else if (d->type == ft_real) {
        retstat = closedir(d->dp);
        if (retstat < 0)
            retstat = -errno;
    } else // invalid type
        retstat = EBADF;
    free(d);

    return retstat;
}

struct ffindex_filep {
    enum FILE_TYPE type;
    int fd;
    size_t data_size;
    char *data;
    char *out;
    size_t length;
    time_t mtime;
};

/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 * Changed in version 2.2
 */
int ffindex_open(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    char fpath[PATH_MAX];

    log_msg("\nffindex_open(path\"%s\", fi=0x%08x)\n", path, fi);

    struct ffindex_filep *d = malloc(sizeof(struct ffindex_filep));
    if (d == NULL)
        return -ENOMEM;

    if (ffindex_fullpath(fpath, path, PATH_MAX) != 0) {
        free(d);
        return -ENAMETOOLONG;
    };

    d->type = ft_real;
    d->fd = open(fpath, fi->flags);
    if (d->fd < 0) {
        retstat = -errno;
        char *dir = strrchr(fpath, '/');
        if (dir != NULL) {
            strcpy(dir, ".idx");
            int fd = open(fpath, O_RDONLY);
            if (fd < 0) {
                retstat = -errno;
            } else {
                char *fname = basename((char *)path);
                ffindex_entry_t* entry = ffindex_get_entry_by_name(fd, fname);
                if (entry == NULL) {
                    retstat = -ENOENT;
                    goto index_close;
                }

                strcpy(dir, ".dat");
                d->fd = open(fpath, O_RDONLY);
                if (d->fd < 0) {
                    retstat = -errno;
                    goto index_free;
                }

                d->data = ffindex_mmap_data2(d->fd, &d->data_size);
                if (d->data == MAP_FAILED || d->data == NULL) {
                    retstat = -errno;
                    close(d->fd);
                } else {
                    d->length = entry->length - 1;
                    d->mtime = entry->mtime;
                    d->out = ffindex_get_data_by_entry(d->data, entry);
                    if(d->out != NULL) {
                        retstat = 0;
                        d->type = ft_virtual;
                        fi->keep_cache = 1;
                        fi->nonseekable = 0;
                    } else {
                        retstat = -ENOENT;
                        ffindex_munmap_data(d->data, d->data_size);
                        close(d->fd);
                    }
                }
index_free:
//                free(entry->name);
                free(entry);
index_close:
                close(fd);
            }
        }
    };

    if (retstat == 0) {
        fi->fh = (unsigned long) d;
        log_fi(fi);
    } else
        free(d);

    return retstat;
}

static inline struct ffindex_filep *get_filep(struct fuse_file_info *fi)
{
    return (struct ffindex_filep *) (uintptr_t) fi->fh;
}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * Changed in version 2.2
 */
// I don't fully understand the documentation above -- it doesn't
// match the documentation for the read() system call which says it
// can return with anything up to the amount of data requested. nor
// with the fusexmp code which returns the amount of data also
// returned by read.
int ffindex_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int retstat = 0;
    struct ffindex_filep *d = get_filep(fi);

    log_msg("\nffindex_read(path=\"%s\", buf=0x%08x, size=%d, offset=%lld, fi=0x%08x)\n", path, buf, size, offset, fi);
    // no need to get fpath on this one, since I work from fi->fh not the path
    log_fi(fi);

    if (d->type == ft_virtual) {
        retstat = size;
        if (offset < d->length) {
            if (retstat > d->length - offset) 
                retstat = d->length - offset;
            memcpy(buf, d->out+offset, retstat);
        } else {
            retstat = 0;
        }
    } else if (d->type == ft_real) {
        retstat = pread(d->fd, buf, size, offset);
        if (retstat < 0)
            retstat = -errno;
    } else // invalid type
        retstat = EBADF;

    return retstat;
}

/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().  This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.  It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 *
 * Changed in version 2.2
 */
int ffindex_flush(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;

    log_msg("\nffindex_flush(path=\"%s\", fi=0x%08x)\n", path, fi);
    // no need to get fpath on this one, since I work from fi->fh not the path
    log_fi(fi);

    return retstat;
}


/**
 * Get attributes from an open file
 *
 * This method is called instead of the getattr() method if the
 * file information is available.
 *
 * Currently this is only called after the create() method if that
 * is implemented (see above).  Later it may be called for
 * invocations of fstat() too.
 *
 * Introduced in version 2.5
 */
// Since it's currently only called after bb_create(), and bb_create()
// opens the file, I ought to be able to just use the fd and ignore
// the path... 
int ffindex_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
    int retstat = 0;
    struct ffindex_filep *d = get_filep(fi);

    log_msg("\nffindex_fgetattr(path=\"%s\", statbuf=0x%08x, fi=0x%08x)\n", path, statbuf, fi);
    log_fi(fi);

    if (d->type == ft_virtual) {
        statbuf->st_mode = S_IFREG | 0444;
        statbuf->st_nlink = 1;
        statbuf->st_size = d->length;
        statbuf->st_ctime = statbuf->st_mtime = statbuf->st_atime = d->mtime;
        statbuf->st_blocks = (statbuf->st_size + 511) / 512;
        statbuf->st_blksize = 4096;
    } else if (d->type == ft_real) {
        retstat = fstat(d->fd, statbuf);
        if (retstat < 0)
            retstat = ffindex_error("ffindex_fgetattr fstat");
    } else // invalid type
        retstat = EBADF;

    log_stat(statbuf);

    return retstat;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int ffindex_release(const char *path, struct fuse_file_info *fi)
{
    int retstat = 0;
    struct ffindex_filep *d = get_filep(fi);

    log_msg("\nffindex_release(path=\"%s\", fi=0x%08x)\n", path, fi);
    log_fi(fi);

    if (d->type == ft_virtual) {
        ffindex_munmap_data(d->data, d->data_size);
    }
    // We need to close the file.  Had we allocated any resources
    // (buffers etc) we'd need to free them here as well.
    retstat = close(d->fd);
    if (retstat < 0)
        retstat = -errno;
    free(d);

    return retstat;
}

struct fuse_operations ffindex_oper = {
  .getattr = ffindex_getattr,
  .fgetattr = ffindex_fgetattr,
  // no .getdir -- that's deprecated
  .getdir = NULL,
//  .flush = ffindex_flush,
  .open = ffindex_open,
  .read = ffindex_read,
  .release = ffindex_release,
  .opendir = ffindex_opendir,
  .readdir = ffindex_readdir,
  .releasedir = ffindex_releasedir,
};

static struct fuse_opt ffindexfs_opts[] = {
     FUSE_OPT_KEY("-V",             KEY_VERSION),
     FUSE_OPT_KEY("-v",             KEY_VERSION),
     FUSE_OPT_KEY("--version",      KEY_VERSION),
     FUSE_OPT_KEY("-h",             KEY_HELP),
     FUSE_OPT_KEY("--help",         KEY_HELP),
     FUSE_OPT_END
};

/**
 * Function to process arguments (called from fuse_opt_parse).
 *
 * @param data  Pointer to fusezip_param structure
 * @param arg is the whole argument or option
 * @param key determines why the processing function was called
 * @param outargs the current output argument list
 * @return -1 on error, 0 if arg is to be discarded, 1 if arg should be kept
 */
static int ffindexfs_opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs)
{
    static int strArgCount = 0;
    int *help_only = data;

    // 'magic' fuse_opt_proc return codes
    const static int KEEP = 1;
    const static int DISCARD = 0;
    const static int ERROR = -1;

    switch (key) {
        case KEY_HELP:
            print_usage(outargs->argv[0]);
            *help_only = 1;
            return DISCARD;
        case KEY_VERSION:
            print_version(outargs->argv[0]);
            *help_only = 1;
            return DISCARD;
        case FUSE_OPT_KEY_NONOPT:
            ++strArgCount;
            switch (strArgCount) {
                case 1:
                    // root directory name
                    rootdir = (char *)arg;
                    return DISCARD;
                case 2:
                    // mountpoint
                    // keep it and then pass to FUSE initializer
                    return KEEP;
                default:
                    fprintf(stderr, "%s: only two arguments allowed: filename and mountpoint\n", basename(outargs->argv[0]));
                    return ERROR;
            }
    }
    return KEEP;
}


int main(int argc, char *argv[])
{
    struct fuse_args ffindexfs_args = FUSE_ARGS_INIT(argc, argv);
    int res;
    int help_only = 0;
    int fuse_stat;

    res = fuse_opt_parse(&ffindexfs_args, &help_only, ffindexfs_opts, ffindexfs_opt_proc);
    if (res == -1) {
        fprintf(stderr, "Error parsing option.\n");
        fuse_opt_free_args(&ffindexfs_args);
        return EXIT_FAILURE;
    }

    // if all work is done inside options parsing... 
    if (help_only) {
        fuse_opt_free_args(&ffindexfs_args);
        return EXIT_SUCCESS;
    }
    if (!rootdir) {
        print_usage(ffindexfs_args.argv[0]);
        fuse_opt_free_args(&ffindexfs_args);
        return EXIT_FAILURE;
    }

//    fprintf(stderr, "about to call fuse_main\n");
    fuse_stat = fuse_main(ffindexfs_args.argc, ffindexfs_args.argv, &ffindex_oper, NULL);
//    fprintf(stderr, "fuse_main returned %d\n", fuse_stat);
    fuse_opt_free_args(&ffindexfs_args);

    return (fuse_stat == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
