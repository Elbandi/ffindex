From: Laszlo Kajan <lkajan@rostlab.org>
Subject: Makefile fixes
  * make Makefile DESTDIR-aware
  * other changes to allow help2man to work better.
  * deinstall -> uninstall
  .
  Upstream is aware but not interested in DESTDIR.
Forwarded: yes

--- a/src/Makefile
+++ b/src/Makefile
@@ -1,15 +1,16 @@
-INSTALL_DIR?=..
-libdir=`([ -d /usr/lib64 ] && echo lib64) || echo lib`
+PREFIX?=/usr
+INSTALL_DIR=$(DESTDIR)$(PREFIX)
+libdir=lib
 LANG=C
-CC=gcc -fopenmp
-#CC=icc -openmp
+CC=gcc
 CFLAGS+=-std=c99 -O2 -g -fPIC -Wall -pedantic
-LDFLAGS=-lffindex -L.
+SOCURRENT=0
+SOVERSION=$(SOCURRENT).1.0
 ifdef FFINDEX_STATIC
 	LDFLAGS+=-static
 	FFINDEX_OBJECTS=libffindex.a
 else
-	FFINDEX_OBJECTS=libffindex.so libffindex.so.0.1 
+	FFINDEX_OBJECTS=libffindex.so libffindex.so.$(SOVERSION)
 endif
 
 #-Wl,--version-script=libffindex.map
@@ -27,36 +28,37 @@
 ffindex_apply: $(FFINDEX_OBJECTS) ffindex_apply.o ffindex.h
 
 ffindex_apply_mpi: $(FFINDEX_OBJECTS) ffindex_apply_mpi.o ffindex.h
-	mpicc $(CFLAGS) -o $@  ffindex_apply_mpi.o $(LDFLAGS)
+	mpicc $(CFLAGS) -o $@  ffindex_apply_mpi.o $(LDFLAGS) -L. -lffindex
 
 ffindex_apply_mpi.o: ffindex_apply_mpi.c ffindex.h
 	mpicc $(CFLAGS) -c ffindex_apply_mpi.c -o $@ 
 
 ffindex_from_fasta: $(FFINDEX_OBJECTS) ffindex_from_fasta.o ffindex.h
-	$(CC) -o $@ ffindex_from_fasta.o $(LDFLAGS) -L. -lffindex
+	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ ffindex_from_fasta.o $(LDFLAGS) -L. -lffindex
 
 ffindex_get: $(FFINDEX_OBJECTS) ffindex_get.o ffindex.h
-	$(CC) -o $@ ffindex_get.o $(LDFLAGS) -L. -lffindex
+	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ ffindex_get.o $(LDFLAGS) -L. -lffindex
 
 ffindex_build: $(FFINDEX_OBJECTS) ffindex_build.o ffindex.h
-	$(CC) -o $@ ffindex_build.o $(LDFLAGS) -L. -lffindex
+	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ ffindex_build.o $(LDFLAGS) -L. -lffindex
 
 ffindex_modify: $(FFINDEX_OBJECTS) ffindex_modify.o ffindex.h
-	$(CC) -o $@ ffindex_modify.o $(LDFLAGS) -L. -lffindex
+	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ ffindex_modify.o $(LDFLAGS) -L. -lffindex
 
 ffindex_unpack: $(FFINDEX_OBJECTS) ffindex_unpack.o ffindex.h
-	$(CC) -o $@ ffindex_unpack.o $(LDFLAGS) -L. -lffindex
+	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ ffindex_unpack.o $(LDFLAGS) -L. -lffindex
 
-libffindex.so.0.1: ffindex.o ffutil.o
-	$(CC) -Wl,-z,defs -shared -Wl,-soname,libffindex.so.0.1 -o libffindex.so.0.1 ffindex.o ffutil.o -lc
+libffindex.so.$(SOVERSION): ffindex.o ffutil.o
+	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -Wl,-z,defstack -shared -Wl,-soname,libffindex.so.$(SOCURRENT) -o libffindex.so.$(SOVERSION) ffindex.o ffutil.o -lc
 
-libffindex.so: libffindex.so.0.1
-	ln -sf libffindex.so.0.1 libffindex.so
+libffindex.so: libffindex.so.$(SOVERSION)
+	ln -sf libffindex.so.$(SOVERSION) libffindex.so.$(SOCURRENT)
+	ln -sf libffindex.so.$(SOVERSION) libffindex.so
 
 libffindex.a: ffindex.o ffutil.o
 	ar rcs libffindex.a ffutil.o ffindex.o 
 
-test:
+check:
 	rm -f /tmp/test.data /tmp/test.ffindex
 	LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" ./ffindex_build -s /tmp/test.data /tmp/test.ffindex ../test/data ../test/data2
 	LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" ./ffindex_get  /tmp/test.data /tmp/test.ffindex a b foo | tee /tmp/test.out
@@ -76,9 +78,11 @@
 	LD_LIBRARY_PATH=".:$(LD_LIBRARY_PATH)" zsh -c "time ./ffindex_get  ../test_big/nr20_11Jan10_a3m_db ../test_big/nr20_11Jan10_a3m_db.ffindex BABBAMIBA.a3m KARDUZEBA.a3m HARPAQIBA.a3m WOQQATEBA.a3m BEBRAQIBA.a3m GIRSAKEBA.a3m NAQTIFIBA.a3m BIWCIQABA.a3m > /dev/null"
 
 clean:
-	rm -f ffindex.o ffutil.o ffindex_build ffindex_build.o ffindex_get ffindex_get.o libffindex.so libffindex.so.0.1 ffindex_modify ffindex_modify.o \
+	rm -f ffindex.o ffutil.o ffindex_build ffindex_build.o ffindex_get ffindex_get.o libffindex.so libffindex.so.$(SOCURRENT) libffindex.so.$(SOVERSION) ffindex_modify ffindex_modify.o \
-		libffindex.a  ffindex_from_fasta.o ffindex_from_fasta ffindex_apply_mpi.o ffindex_apply.o
+		libffindex.a  ffindex_from_fasta.o ffindex_from_fasta ffindex_apply_mpi.o ffindex_apply.o ffindex_apply ffindex_unpack.o ffindex_unpack
 
+distclean: clean
+
 install:
 	mkdir -p $(INSTALL_DIR)/bin
 	mkdir -p $(INSTALL_DIR)/$(libdir)
@@ -89,9 +93,10 @@
 	install ffindex_from_fasta $(INSTALL_DIR)/bin/ffindex_from_fasta
 	install ffindex_apply $(INSTALL_DIR)/bin/ffindex_apply
 	install ffindex_unpack $(INSTALL_DIR)/bin/ffindex_unpack
-	install libffindex.so.0.1 $(INSTALL_DIR)/$(libdir)/libffindex.so.0.1
+	install libffindex.so.$(SOVERSION) $(INSTALL_DIR)/$(libdir)/libffindex.so.$(SOVERSION)
 	install libffindex.a $(INSTALL_DIR)/$(libdir)/libffindex.a
-	ln -sf libffindex.so.0.1 $(INSTALL_DIR)/$(libdir)/libffindex.so
+	ln -sf libffindex.so.$(SOVERSION) $(INSTALL_DIR)/$(libdir)/libffindex.so.$(SOCURRENT)
+	ln -sf libffindex.so.$(SOVERSION) $(INSTALL_DIR)/$(libdir)/libffindex.so
 	install ffindex.h $(INSTALL_DIR)/include/ffindex.h
 	install ffutil.h $(INSTALL_DIR)/include/ffutil.h
 ifdef HAVE_MPI
--- a/src/ffindex_build.c
+++ b/src/ffindex_build.c
@@ -29,9 +29,9 @@
 #define MAX_FILENAME_LIST_FILES 4096
 
 
-void usage(char *program_name)
+void usage()
 {
-    fprintf(stderr, "USAGE: %s [-a|-v] [-s] [-f file]* OUT_DATA_FILE OUT_INDEX_FILE [-d 2ND_DATA_FILE -i 2ND_INDEX_FILE] [DIR_TO_INDEX|FILE]*\n"
+    fprintf(stderr, "USAGE: ffindex_build [-a|-v] [-s] [-f file]* OUT_DATA_FILE OUT_INDEX_FILE [-d 2ND_DATA_FILE -i 2ND_INDEX_FILE] [DIR_TO_INDEX|FILE]*\n"
                     "\t-a\t\tappend files/indexes, also needed for sorting an already existing ffindex\n"
                     "\t-d FFDATA_FILE\ta second ffindex data file for inserting/appending\n"
                     "\t-i FFINDEX_FILE\ta second ffindex index file for inserting/appending\n"
@@ -52,7 +52,7 @@ void usage(char *program_name)
                     "\tMaximum key/filename length is %d and maximum entries are by default %d\n"
                     "\tThis can be changed in the sources.\n"
                     "\nDesigned and implemented by Andreas W. Hauser <hauser@genzentrum.lmu.de>.\n",
-                    program_name, MAX_FILENAME_LIST_FILES, FFINDEX_MAX_ENTRY_NAME_LENTH, FFINDEX_MAX_INDEX_ENTRIES_DEFAULT);
+                    MAX_FILENAME_LIST_FILES, FFINDEX_MAX_ENTRY_NAME_LENTH, FFINDEX_MAX_INDEX_ENTRIES_DEFAULT);
 }
 
 int main(int argn, char **argv)
@@ -88,7 +88,7 @@ int main(int argn, char **argv)
         version = 1;
         break;
       default:
-        usage(argv[0]);
+        usage();
         return EXIT_FAILURE;
     }
   }
@@ -102,7 +102,7 @@ int main(int argn, char **argv)
 
   if(argn - optind < 2)
   {
-    usage(argv[0]);
+    usage();
     return EXIT_FAILURE;
   }
 
--- a/src/ffindex_modify.c
+++ b/src/ffindex_modify.c
@@ -28,16 +28,16 @@
 
 #define MAX_FILENAME_LIST_FILES 4096
 
-void usage(char *program_name)
+void usage()
 {
-    fprintf(stderr, "USAGE: %s [-s|-u|-v] [-t] [-f file]* index_filename [filename]*\n"
+    fprintf(stderr, "USAGE: ffindex_modify [-s|-u|-v] [-t] [-f file]* index_filename [filename]*\n"
                     "\t-f file\tfile each line containing a filename\n"
                     "\t\t-f can be specified up to %d times\n"
                     "\t-s\tsort index file\n"
                     "\t-u\tunlink entry (remove from index only)\n"
                     "\t-v\tprint version and other info then exit\n"
                     "\nDesigned and implemented by Andreas W. Hauser <hauser@genzentrum.lmu.de>.\n",
-                    program_name, MAX_FILENAME_LIST_FILES);
+                    MAX_FILENAME_LIST_FILES);
 }
 
 int main(int argn, char **argv)
@@ -67,7 +67,7 @@ int main(int argn, char **argv)
         break;
       default:
         fprintf(stderr, "Option %c not recognized\n", opt);
-        usage(argv[0]);
+        usage();
         return EXIT_FAILURE;
     }
   }
@@ -81,7 +81,7 @@ int main(int argn, char **argv)
 
   if(optind >= argn)
   {
-    usage(argv[0]);
+    usage();
     return EXIT_FAILURE;
   }
 
@@ -148,14 +148,9 @@
         sorted_names_to_unlink[y] = argv[i];
       ffindex_unlink_entries(index, sorted_names_to_unlink, y);
 
-      /* Sort the index entries and write back */
+      /* Sort the index entries */
       if(sort)
-      {
         ffindex_sort_index_file(index);
-        index_file = fopen(index_filename, "w");
-        if(index_file == NULL) { perror(index_filename); return EXIT_FAILURE; }
-        err += ffindex_write(index, index_file);
-      }
     }
   }
 
--- a/src/ffindex_get.c
+++ b/src/ffindex_get.c
@@ -22,12 +22,11 @@
 #include "ffindex.h"
 #include "ffutil.h"
 
-void usage(char* program_name)
+void usage()
 {
-    fprintf(stderr, "USAGE: %s data_filename index_filename entry name(s)\n"
+    fprintf(stderr, "USAGE: ffindex_get data_filename index_filename entry name(s)\n"
                     "-n\tuse index of entry instead of entry name\n"
-                    "\nDesigned and implemented by Andy Hauser <hauser@genzentrum.lmu.de>.\n",
-                    program_name);
+                    "\nDesigned and implemented by Andy Hauser <hauser@genzentrum.lmu.de>.\n");
 }
 
 int main(int argn, char **argv)
@@ -42,13 +41,13 @@ int main(int argn, char **argv)
         by_index = 1;
         break;
       default:
-        usage(argv[0]);
+        usage();
         return EXIT_FAILURE;
     }
   }
   if(argn < 3)
   {
-    usage(argv[0]);
+    usage();
     return EXIT_FAILURE;
   }
   char *data_filename  = argv[optind++];
--- a/src/ffindex_apply.c
+++ b/src/ffindex_apply.c
@@ -33,9 +33,9 @@
 {
   if(argn < 4)
   {
-    fprintf(stderr, "USAGE: %s DATA_FILENAME INDEX_FILENAME PROGRAM [PROGRAM_ARGS]*\n"
-                    "\nDesigned and implemented by Andy Hauser <hauser@genzentrum.lmu.de>.\n",
-                    argv[0]);
+    fprintf(stderr, "USAGE: ffindex_apply DATA_FILENAME INDEX_FILENAME PROGRAM [PROGRAM_ARGS]*\n"
+                    "\nDesigned and implemented by Andy Hauser <hauser@genzentrum.lmu.de>.\n"
+    );
     return -1;
   }
   char *data_filename  = argv[1];
--- a/src/ffindex_from_fasta.c
+++ b/src/ffindex_from_fasta.c
@@ -29,11 +29,11 @@
 #define MAX_FILENAME_LIST_FILES 4096
 
 
-void usage(char *program_name)
+void usage()
 {
-    fprintf(stderr, "USAGE: %s -v | [-s] data_filename index_filename fasta_filename\n"
+    fprintf(stderr, "USAGE: ffindex_from_fasta -v | [-s] data_filename index_filename fasta_filename\n"
                     "\t-s\tsort index file\n"
-                    "\nDesigned and implemented by Andreas W. Hauser <hauser@genzentrum.lmu.de>.\n", program_name);
+                    "\nDesigned and implemented by Andreas W. Hauser <hauser@genzentrum.lmu.de>.\n");
 }
 
 int main(int argn, char **argv)
@@ -51,7 +51,7 @@ int main(int argn, char **argv)
         version = 1;
         break;
       default:
-        usage(argv[0]);
+        usage();
         return EXIT_FAILURE;
     }
   }
@@ -65,7 +65,7 @@ int main(int argn, char **argv)
 
   if(argn - optind < 3)
   {
-    usage(argv[0]);
+    usage();
     return EXIT_FAILURE;
   }
 
diff --git a/src/ffindex_unpack.c b/src/ffindex_unpack.c
index 38a347f..2473202 100644
--- a/src/ffindex_unpack.c
+++ b/src/ffindex_unpack.c
@@ -33,9 +33,9 @@ int main(int argn, char **argv)
 {
   if(argn < 4)
   {
-    fprintf(stderr, "USAGE: %s DATA_FILENAME INDEX_FILENAME OUT_DIR\n"
-                    "\nDesigned and implemented by Andy Hauser <hauser@genzentrum.lmu.de>.\n",
-                    argv[0]);
+    fprintf(stderr, "USAGE: ffindex_unpack DATA_FILENAME INDEX_FILENAME OUT_DIR\n"
+                    "\nDesigned and implemented by Andy Hauser <hauser@genzentrum.lmu.de>.\n"
+    );
     return -1;
   }
   char *data_filename  = argv[1];
