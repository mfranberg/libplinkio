AM_CFLAGS = $(PYTHON_INCLUDE) -I$(srcdir)/../src/plinkio -I$(srcdir)/../src/

pkgpythondir = $(pythondir)/plinkio
pkgpyexecdir = $(pyexecdir)/plinkio

pkgpython_SCRIPTS = wrapper/__init__.py wrapper/plinkfile.py
pkgpython_LTLIBRARIES = cplinkio.la

cplinkio_la_SOURCES = cplinkio.c snparray.c snparray.h common.h
cplinkio_la_LIBADD = ../src/libplinkio.la
cplinkio_la_LDFLAGS = -avoid-version -module

EXTRA_DIST = wrapper/__init__.py wrapper/plinkfile.py
