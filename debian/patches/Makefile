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
 		libffindex.a  ffindex_from_fasta.o ffindex_from_fasta ffindex_apply_mpi.o ffindex_apply.o ffindex_apply ffindex_unpack.o ffindex_unpack
 
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
--- a/src/ffindex_modify.c
+++ b/src/ffindex_modify.c
 int main(int argn, char **argv)
@@ -148,14 +148,9 @@ int main(int argn, char **argv)
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
 
