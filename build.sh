#!/bin/bash

setup_build_env.sh

./build_giflib.sh

source common.sh

VERSION=`cat version`

./gen.py

pushd wkhtmltopdf/static-build/centos
./build.sh
popd

wkhtmltopdf/static-build/centos/qt/bin/qmake ichabod.pro
make

archive_src.sh

build_rpm.sh
