lib_LTLIBRARIES=liblogc.la
liblogc_la_SOURCES=logc.h logc.c
liblogc_la_CFLAGS=${CFLAGS} -DBINARY_COLS=${BINARY_COLS}
#liblogc_la_LDFLAGS=-shared -fpic
include_HEADERS=logc.h
ACLOCAL_AMFLAGS=-I m4

check_PROGRAMS=liblogc_test
liblogc_test_SOURCES=liblogc_test.c
liblogc_test_LDADD=$(top_builddir)/liblogc.la
liblogc_test_LDFLAGS=-static
