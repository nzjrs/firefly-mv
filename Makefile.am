AM_CFLAGS = $(DC1394_CFLAGS)
AM_CFLAGS += $(GLIB_CFLAGS)

LIBS += $(DC1394_LIBS)
LIBS += $(GLIB_LIBS)

LDADD = libutil.la

pkglib_LTLIBRARIES = libutil.la

bin_PROGRAMS = dc1394-camls dc1394-record

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = firefly-mv-utils.pc

if ENABLE_GTK
pkglib_LTLIBRARIES += libgtkutil.la
pkgconfig_DATA += firefly-mv-utils-gtk.pc
bin_PROGRAMS +=     \
    dc1394-play     \
    dc1394-show     \
    dc1394-view     \
    dc1394-save
endif

if ENABLE_OPENCV
pkglib_LTLIBRARIES += libopencvutil.la
pkgconfig_DATA += firefly-mv-utils-opencv.pc
bin_PROGRAMS +=     \
    dc1394-opencv-view
endif

libutil_ladir = $(pkgincludedir)
libutil_la_SOURCES = utils.c
libutil_la_CFLAGS = $(GLIB_CFLAGS)
libutil_la_HEADERS = utils.h

libgtkutil_ladir = $(pkgincludedir)
libgtkutil_la_SOURCES = gtkutils.c
libgtkutil_la_CFLAGS = $(GTK_CFLAGS)
libgtkutil_la_HEADERS = gtkutils.h

libopencvutil_ladir = $(pkgincludedir)
libopencvutil_la_SOURCES = opencvutils.c
libopencvutil_la_CFLAGS = $(OPENCV_CFLAGS)
libopencvutil_la_HEADERS = opencvutils.h

dc1394_camls_SOURCES = camls.c

dc1394_record_SOURCES = record.c

dc1394_play_SOURCES = play.c
dc1394_play_CFLAGS = $(GTK_CFLAGS)
dc1394_play_LDADD = $(GTK_LIBS) libgtkutil.la libutil.la

dc1394_show_SOURCES = show.c
dc1394_show_CFLAGS = $(GTK_CFLAGS)
dc1394_show_LDADD = $(GTK_LIBS) libgtkutil.la libutil.la

dc1394_view_SOURCES = view.c
dc1394_view_CFLAGS = $(GTK_CFLAGS)
dc1394_view_LDADD = $(GTK_LIBS) libgtkutil.la libutil.la

dc1394_save_SOURCES = save-to-images.c
dc1394_save_CFLAGS = $(GTK_CFLAGS)
dc1394_save_LDADD = $(GTK_LIBS) libgtkutil.la libutil.la

dc1394_opencv_view_SOURCES = opencvview.c
dc1394_opencv_view_CFLAGS = $(OPENCV_CFLAGS)
dc1394_opencv_view_LDADD = $(OPENCV_LIBS) libopencvutil.la libutil.la

