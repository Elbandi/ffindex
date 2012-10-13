dnl $Id$
dnl config.m4 for extension couchbase

PHP_ARG_WITH(ffindex, for ffindex support,
dnl Make sure that the comment is aligned:
[  --with-ffindex=[DIR]            Set the path to libffindex install prefix])

if test "$PHP_FFINDEX" != "no"; then
  if test -r "$PHP_FFINDEX/ffindex.h"; then
      AC_MSG_CHECKING([for libffindex location])
      FFINDEX_DIR="$PHP_FFINDEX"
      FFINDEX_INCDIR="$FFINDEX_DIR"
      FFINDEX_LIBDIR="$FFINDEX_DIR"
      AC_MSG_RESULT($PHP_FFINDEX)
  else
    dnl # look in system dirs
     AC_MSG_CHECKING([for libffindex files in default path])
    for i in /usr /usr/local /opt/local; do
      if test -r "$i/include/ffindex.h"; then
        FFINDEX_DIR=$i
        FFINDEX_INCDIR="$FFINDEX_DIR/include"
        FFINDEX_LIBDIR="$FFINDEX_DIR/lib"
        AC_MSG_RESULT(found in $i)
        break
      fi
    done
  fi

  if test -z "$FFINDEX_DIR"; then
     AC_MSG_RESULT([not found])
     AC_MSG_ERROR([ffindex support requires libffindex. Use --with-ffindex=<DIR> to specify the prefix where libffindex headers and library are located])
  fi

  AC_DEFINE(HAVE_FFINDEXLIB,1,[ ])
  PHP_ADD_LIBRARY_WITH_PATH(ffindex, $FFINDEX_LIBDIR, FFINDEX_SHARED_LIBADD)
  PHP_SUBST(FFINDEX_SHARED_LIBADD)

  PHP_NEW_EXTENSION(ffindex, ffindex-php.c, $ext_shared)
  PHP_ADD_INCLUDE($FFINDEX_INCDIR)
fi
