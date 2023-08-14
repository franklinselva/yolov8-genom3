
lib_LTLIBRARIES = libYolov8_codels.la

libYolov8_codels_la_SOURCES  =	Yolov8_c_types.h
libYolov8_codels_la_SOURCES +=	Yolov8_codels.cc
libYolov8_codels_la_SOURCES +=	Yolov8_detect_objects_codels.cc

libYolov8_codels_la_CPPFLAGS =	$(requires_CFLAGS)
libYolov8_codels_la_LIBADD   =	$(requires_LIBS)
libYolov8_codels_la_CPPFLAGS+=	$(codels_requires_CFLAGS)
libYolov8_codels_la_LIBADD  +=	$(codels_requires_LIBS)
libYolov8_codels_la_LDFLAGS  =	-release $(PACKAGE_VERSION)


# idl  mappings
BUILT_SOURCES=	Yolov8_c_types.h
CLEANFILES=	${BUILT_SOURCES}

Yolov8_c_types.h: ${top_srcdir}/yolov8.gen
	${GENOM3}  mappings \
	  -MD -MF .deps/$@.d -MT $@ --signature -l c $< >$@

-include .deps/Yolov8_c_types.h.d
