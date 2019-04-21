ACLOCAL_AMFLAGS = -I m4

SUBDIRS        = src
EXTRA_DIST     = get-version autogen.sh

dist-hook:
	echo $(VERSION) > $(distdir)/.dist-version
