[+ autogen5 template +]
[+
(define prefix_if_missing
        (lambda
                (name prefix)
                (string-append
                         (if
                                (==* (get name) prefix)
                                ""
                                prefix
                        )
                        (get name)
                )
        )
)
+]## Process this file with automake to produce Makefile.in

## Created by Anjuta

AM_CPPFLAGS = \
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\" \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"[+IF (=(get "HavePackage") "1")+] \
	$([+NameCUpper+]_CFLAGS)[+ENDIF+]

AM_CFLAGS =\
	 -Wall\
	 -g

lib_LTLIBRARIES = [+(prefix_if_missing "NameHLower" "lib")+].la

vapidir = $(datadir)/vala/vapi
dist_vapi_DATA = [+NameHLower+]-[+Version+].vapi [+NameHLower+]-[+Version+].deps

[+(prefix_if_missing "NameCLower" "lib")+]_la_SOURCES = [+NameHLower+].vala

[+(prefix_if_missing "NameCLower" "lib")+]_la_LDFLAGS = 
	
[+(prefix_if_missing "NameCLower" "lib")+]_la_VALAFLAGS = [+IF (not (= (get "PackageModule2") ""))+] --pkg [+(string-substitute (get "PackageModule2") " " " --pkg ")+] [+ENDIF+] \
	--library [+NameHLower+]-[+Version+] --vapi [+NameHLower+]-[+Version+].vapi -H [+NameHLower+].h

[+(prefix_if_missing "NameCLower" "lib")+]_la_LIBADD = [+IF (=(get "HavePackage") "1")+]$([+NameCUpper+]_LIBS)[+ENDIF+]

[+IF (=(get "HaveWindowsSupport") "1")+]
if PLATFORM_WIN32
[+(prefix_if_missing "NameCLower" "lib")+]_la_LDFLAGS += -no-undefined
endif

if NATIVE_WIN32
[+(prefix_if_missing "NameCLower" "lib")+]_la_LDFLAGS += -export-dynamic
endif[+
ENDIF+]

[+NameHLower+]includedir = $(includedir)/[+NameHLower+]-[+Version+]
[+NameHLower+]include_HEADERS = \
	[+NameHLower+].h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = [+NameHLower+]-[+Version+].pc

EXTRA_DIST = \
	[+NameHLower+]-[+Version+].pc.in
