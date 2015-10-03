#!/bin/sh
# Run this to generate all the initial makefiles, etc.


test -z "$srcdir" && srcdir=.

GNOMEDOC=`which yelp-build`
if test -z $GNOMEDOC; then
        echo "*** The tools to build the documentation are not found,"
        echo "    please intall the yelp-tool package ***"
        exit 1
fi

srcdir=`dirname $0`
echo "Generating initial interface files"
sh -c "cd $srcdir/libanjuta/interfaces && \
perl anjuta-idl-compiler.pl libanjuta && \
touch iface-built.stamp"

test -n "$srcdir" || srcdir=`dirname "$0"`
test -n "$srcdir" || srcdir=.
(
 cd "$srcdir" &&
 gtkdocize &&
 autopoint --force &&
 AUTOPOINT='intltoolize --automake --copy' autoreconf --force --install
) || exit
test -n "$NOCONFIGURE" || "$srcdir/configure" "$@"
