# Makefile for ZipArchive library
# Copyright (C) 2000 - 2009 Artpol Software - Tadeusz Dracz
# For conditions of distribution and use, see copyright notice in License.txt
# To install to /usr/lib and /usr/include, type:
#    make install
# to install to a different directory change prefix


#Define this variable to use the bzip2 library provided with the ZipArchive Library
#otherwise, the system's bzip2 library will be used
#INTERNAL_BZIP2 = 1

CC=g++

CCC=cc
#CCC=gcc
	

CFLAGS = -D _ZIP_SYSTEM_LINUX 
#CFLAGS =

ifdef INTERNAL_BZIP2
CFLAGS += -D _ZIP_BZIP2_INTERNAL
endif

ZIPARCHLIB = libZipArchive.a

ZIPPIELOCATION = ../Zippie/

ZLIBLOCATION = zlib/

ifdef INTERNAL_BZIP2
	BZIP2LOCATION = bzip2/
endif

INCLUDES = -I. -Izlib

ifdef INTERNAL_BZIP2
INCLUDES += -Ibzip2
endif

prefix = /usr
#prefix = 

libdir = ${prefix}/lib
includedir = ${prefix}/include
zipardir = $(includedir)/ziparchive
zlibdir = $(zipardir)/zlib

ifdef INTERNAL_BZIP2
	bzip2dir = $(zipardir)/bzip2
endif

AR=ar rc
RANLIB=ranlib

.SUFFIXES: .c .cpp
.c.o:
	$(CCC) $(CFLAGS) $(INCLUDES) -c -o $*.o $<
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

OBJS = $(ZLIBLOCATION)adler32.o $(ZLIBLOCATION)compress.o $(ZLIBLOCATION)crc32.o $(ZLIBLOCATION)uncompr.o $(ZLIBLOCATION)deflate.o $(ZLIBLOCATION)trees.o \
$(ZLIBLOCATION)zutil.o $(ZLIBLOCATION)inflate.o $(ZLIBLOCATION)infback.o $(ZLIBLOCATION)inftrees.o $(ZLIBLOCATION)inffast.o\
ZipArchive.o ZipAutoBuffer.o ZipCentralDir.o \
ZipCompressor.o BaseLibCompressor.o Bzip2Compressor.o DeflateCompressor.o ZipCompatibility.o ZipException.o ZipFile_stl.o ZipFileHeader.o \
ZipMemFile.o ZipPlatformComm.o \
ZipStorage.o ZipString.o ZipExtraData.o ZipExtraField.o \
DirEnumerator.o FileFilter.o Wildcard.o \
ZipCryptograph.o ZipCrc32Cryptograph.o \
Aes.o Hmac.o RandomPool.o ZipAesCryptograph.o Sha1.o

ifdef INTERNAL_BZIP2
	OBJS += $(BZIP2LOCATION)bzlib.o $(BZIP2LOCATION)blocksort.o $(BZIP2LOCATION)bzcompress.o $(BZIP2LOCATION)crctable.o\
	$(BZIP2LOCATION)decompress.o $(BZIP2LOCATION)huffman.o $(BZIP2LOCATION)randtable.o
endif

OBJP = ZipPathComponent_lnx.o ZipPlatform_lnx.o
#OBJP = ZipPathComponent_win.o ZipPlatform_win.o

$(ZIPARCHLIB): $(OBJS) $(OBJP) 
	$(AR) $@ $(OBJS) $(OBJP)
	-@ ($(RANLIB) $@ || true) >/dev/null 2>&1

adler32.o: zlib.h zconf.h
compress.o: zlib.h zconf.h
crc32.o: crc32.h zlib.h zconf.h
deflate.o: deflate.h zutil.h zlib.h zconf.h
inffast.o: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h
inflate.o: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h
infback.o: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h
inftrees.o: zutil.h zlib.h zconf.h inftrees.h
trees.o: deflate.h zutil.h zlib.h zconf.h trees.h
uncompr.o: zlib.h zconf.h
zutil.o: zutil.h zlib.h zconf.h

ifdef INTERNAL_BZIP2
	bzlib.o: bzlib.h bzlib_private.h
	blocksort.o: bzlib_private.h
	bzcompress.o: bzlib_private.h
	crctable.o: bzlib_private.h
	decompress.o: bzlib_private.h
	huffman.o: bzlib_private.h
	randtable.o: bzlib_private.h
endif

ZipArchive.o: ZipArchive.h
ZipAutoBuffer.o: ZipAutoBuffer.h
ZipCentralDir.o: ZipCentralDir.h
ZipCompressor.o: ZipCompressor.h
BaseLibCompressor.o: BaseLibCompressor.h
Bzip2Compressor.o: Bzip2Compressor.h
DeflateCompressor.o: DeflateCompressor.h
ZipCompatibility.o: ZipCompatibility.h
ZipException.o: ZipException.h
ZipFile_stl.o: ZipFile.h
ZipFileHeader.o: ZipFileHeader.h
ZipMemFile.o: ZipMemFile.h
ZipStorage.o: ZipStorage.h
ZipString.o: ZipString.h
ZipExtraData.o: ZipExtraData.h
ZipExtraField.o: ZipExtraField.h
DirEnumerator.o: DirEnumerator.cpp
FileFilter.o: FileFilter.cpp
Wildcard.o: Wildcard.cpp
ZipCryptograph.o: ZipCryptograph.h
ZipCrc32Cryptograph.o: ZipCrc32Cryptograph.h
Aes.o: Aes.h
Hmac.o: Hmac.h
RandomPool.o: RandomPool.h
ZipAesCryptograph.o: ZipAesCryptograph.h
Sha1.o: Sha1.h

ZipPathComponent_lnx.o: ZipPathComponent.h
#ZipPathComponent_win.o: ZipPathComponent.h

ZipPlatform_lnx.o: ZipPlatform.h
#ZipPlatform_win.o: ZipPlatform.h


LIBS = -lstdc++ -lziparch

ifndef INTERNAL_BZIP2
	LIBS += -lbz2
endif

clean:
	-rm -f *.o *~ $(ZIPARCHLIB) $(ZLIBLOCATION)*.o $(ZLIBLOCATION)*~ $(BZIP2LOCATION)*.o $(BZIP2LOCATION)*~

zippie:
	$(CC) -I$(zipardir) $(CFLAGS) -o zippie $(ZIPPIELOCATION)zippie.cpp $(ZIPPIELOCATION)CmdLine.cpp $(LIBS)
	#$(CC) -I$(zipardir) -L$(libdir) $(CFLAGS) -o zippie $(ZIPPIELOCATION)zippie.cpp $(ZIPPIELOCATION)CmdLine.cpp $(LIBS)

cleanzippie:
	-rm -f zippie
	#-rm -f zippie.exe

install:
	-@if [ ! -d $(includedir)  ]; then mkdir $(includedir); fi
	-@if [ ! -d $(libdir) ]; then mkdir $(libdir); fi
	-@if [ ! -d $(zipardir)  ]; then mkdir $(zipardir); fi
	-@if [ ! -d $(zlibdir)  ]; then mkdir $(zlibdir); fi
	cp libziparch.a $(libdir)
	chmod 755 $(libdir)/$(ZIPARCHLIB)
	cp *.h $(zipardir)
	chmod 644 $(zipardir)/*h
	cp $(ZLIBLOCATION)*.h $(zlibdir)
	chmod 644 $(zlibdir)/*h
ifdef INTERNAL_BZIP2	
	-@if [ ! -d $(bzip2dir)  ]; then mkdir $(bzip2dir); fi
	cp $(BZIP2LOCATION)*.h $(bzip2dir)
	chmod 644 $(bzip2dir)/*h
endif	
	
uninstall:
	rm -f $(zlibdir)/*h	
	rm -f $(zipardir)/*h	
	rm -f $(libdir)/$(ZIPARCHLIB)
	rmdir $(zlibdir)		
	
ifdef INTERNAL_BZIP2	
	rm -f $(bzip2dir)/*h
	rmdir $(bzip2dir)
endif
	rmdir $(zipardir)



