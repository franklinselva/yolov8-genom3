
ACLOCAL_AMFLAGS=	-I autoconf

# IDL source files
idldir=		$(datadir)/idl/Yolov8
nobase_dist_idl_DATA= yolov8.gen

pkgconfigdir=	$(libdir)/pkgconfig
pkgconfig_DATA=
pkgconfig_DATA+= Yolov8-genom3.pc

# we don't want generated templates in the distribution
#
DIST_SUBDIRS=		codels
SUBDIRS=		${DIST_SUBDIRS}

# recursion into templates directories configured with --with-templates
#
SUBDIRS+=		${AG_TEMPLATES_SUBDIRS}

distclean-local:
	-rm -rf ${AG_TEMPLATES_SUBDIRS}

# a rule to invoke skeleton merge mode
#
merge: merge-interactive
merge-%:
	cd ${top_srcdir} && ${GENOM3}  \
		skeleton -l 'c++' -m $* yolov8.gen

# documentation
dist_doc_DATA=	README.html README.adoc

%.html: %.adoc
	$(ASCIIDOCTOR) $(ASCIIDOCTOR_ARGS) -b html5 -o $@ $<
