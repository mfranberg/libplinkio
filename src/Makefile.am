if GCC
AM_CFLAGS = -Wall -strict -pendantic
endif

lib_LTLIBRARIES = libplinkio.la
libplinkio_la_HEADERS = $(top_srcdir)/src/plinkio/*.h
libplinkio_la_SOURCES = fam.c fam_parse.c bim.c bed.c plinkio.c file.c bed_header.c
libplinkio_ladir = $(includedir)/plinkio
libplinkio_la_CFLAGS = -I$(top_srcdir)/src/plinkio -I$(top_srcdir)/libs/libcsv/inc
libplinkio_la_LIBADD = ../libs/libcsv/libcsv.la
