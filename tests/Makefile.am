AM_CFLAGS = -I$(srcdir)/../libs/cmockery/src/google -I$(srcdir)/../src/plinkio -I$(srcdir)/../src/ -I$(srcdir)/../libs/libcsv/inc -DUNIT_TESTING=1

if GCC
AM_CFLAGS += -Wall
endif

TESTS = fam_test bim_test bed_test
noinst_PROGRAMS = $(TESTS) plinkio_test transpose

fam_test_SOURCES = fam_test.c mock.c
fam_test_LDADD = ../libs/cmockery/libcmockery.la ../libs/libcsv/libcsv.la

bim_test_SOURCES = bim_test.c mock.c
bim_test_LDADD = ../libs/cmockery/libcmockery.la ../libs/libcsv/libcsv.la


bed_test_SOURCES = bed_test.c
bed_test_LDADD = ../libs/cmockery/libcmockery.la

plinkio_test_SOURCES = plinkio_test.c ../libs/libcsv/libcsv.la
plinkio_test_LDADD = ../src/libplinkio.la
plinkio_test_LDFLAGS = -static

transpose_SOURCES = transpose.c
transpose_LDADD = ../src/libplinkio.la -lm
transpose_LDFLAGS = -static
