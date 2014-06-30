[+ autogen5 template +]
#!/usr/bin/env python
[+INCLUDE (string-append "licenses/" (get "License") ".tpl") \+]
[+INCLUDE (string-append "indent.tpl") \+]
# [+INVOKE EMACS-MODELINE MODE="Python; coding: utf-8" +]
[+INVOKE START-INDENT\+]
#
# main.py
# Copyright (C) [+(shell "date +%Y")+] [+Author+][+IF (get "Email")+] <[+Email+]>[+ENDIF+]
# 
[+INVOKE LICENSE-DESCRIPTION PFX="# " PROGRAM=(get "Name") OWNER=(get "Author") \+]

print "Hello World!"
[+INVOKE END-INDENT\+]
