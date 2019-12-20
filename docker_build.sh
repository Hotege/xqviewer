#!/usr/bin/bash

./make.sh
docker rmi hotege/xqviewer
docker build -t hotege/xqviewer .
