include $(top_srcdir)/gtester.mk

AM_CPPFLAGS = $(LIBANJUTA_CFLAGS)

noinst_PROGRAMS += $(TEST_PROGS)

TEST_PROGS += \
	testgit

testgit_SOURCES = \
	testgit.c   \
	../git-status-factory.c \
	../git-status-factory.h \
	../git-status.c \
	../git-status.h 

testgit_LDADD = $(LIBANJUTA_LIBS) $(ANJUTA_LIBS) 