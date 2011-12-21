AM_CFLAGS = -I$(srcdir)/../libs/cmockery/src/google -I$(srcdir)/../src/plinkio -I$(srcdir)/../src/ -DUNIT_TESTING=1

if GCC
AM_CFLAGS += -Wall
endif

TESTS = fam_test bim_test bed_test plinkio_test
noinst_PROGRAMS = $(TESTS)

fam_test_SOURCES = fam_test.c
fam_test_LDADD = ../libs/cmockery/libcmockery.la

bim_test_SOURCES = bim_test.c
bim_test_LDADD = ../libs/cmockery/libcmockery.la

bed_test_SOURCES = bed_test.c
bed_test_LDADD = ../libs/cmockery/libcmockery.la

plinkio_test_SOURCES = plinkio_test.c
plinkio_test_LDADD = ../src/libplinkio.la
plinkio_test_LDFLAGS = -static

