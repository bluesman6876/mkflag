## Process this file with automake to produce Makefile.in

AM_CFLAGS = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\"\
	$(GDKLIB_CFLAGS)\
	-Wall\
	-g

bin_PROGRAMS = mkflag

mkflag_SOURCES = \
	mkflag.c

mkflag_LDFLAGS = 

mkflag_LDADD = \
	$(GDKLIB_LIBS)

EXTRA_DIST = \
	$(flags_DATA)
