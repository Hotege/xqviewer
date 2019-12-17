#!/usr/bin/bash

rm -f xqviewer

gcc server.c \
-o xqviewer \
-I ~/opensources/lua-5.3.5/install/include \
-L ~/opensources/lua-5.3.5/install/lib \
-I ~/opensources/curl-7.67.0/install/include \
-L ~/opensources/curl-7.67.0/install/lib \
-llua -lm -lcurl
