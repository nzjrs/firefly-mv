CFLAGS += $(DC1394_CFLAGS)
CFLAGS += $(GLIB_CFLAGS)

LIBS += $(DC1394_LIBS)
LIBS += $(GLIB_LIBS)

LDADD = libutil.a

noinst_LIBRARIES = libutil.a
bin_PROGRAMS = dc1394-camls dc1394-record

if ENABLE_GTK
noinst_LIBRARIES += libgtkutil.a
bin_PROGRAMS +=     \
    dc1394-play     \
    dc1394-show     \
    dc1394-view
endif

libutil_a_SOURCES = utils.c
libutil_a_CFLAGS = $(GLIB_CFLAGS)

libgtkutil_a_SOURCES = gtkutils.c
libgtkutil_a_CFLAGS = $(GTK_CFLAGS)

dc1394_camls_SOURCES = camls.c

dc1394_record_SOURCES = record.c

dc1394_play_SOURCES = play.c
dc1394_play_CFLAGS = $(GTK_CFLAGS)
dc1394_play_LDADD = $(GTK_LIBS) libgtkutil.a libutil.a

dc1394_show_SOURCES = show.c
dc1394_show_CFLAGS = $(GTK_CFLAGS)
dc1394_show_LDADD = $(GTK_LIBS) libgtkutil.a libutil.a

dc1394_view_SOURCES = view.c
dc1394_view_CFLAGS = $(GTK_CFLAGS)
dc1394_view_LDADD = $(GTK_LIBS) libgtkutil.a libutil.a
