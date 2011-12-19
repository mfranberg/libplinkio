if GCC
AM_CFLAGS = -Wall
endif

lib_LTLIBRARIES = libplinkio.la
libplinkio_la_HEADERS = $(top_srcdir)/src/plinkio/*.h
libplinkio_la_SOURCES = fam.c bim.c
libplinkio_ladir = $(includedir)/plinkio
libplinkio_la_CFLAGS = -I$(top_srcdir)/src/plinkio

