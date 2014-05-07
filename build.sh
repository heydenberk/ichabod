#!/bin/bash

set -e
function error_handler() {
    echo "Error during build!"
    exit 666
}
 
trap error_handler EXIT

./setup_build_env.sh

# linking directly to source for now
#./build_giflib.sh

source common.sh
subm

./gen.py

pushd wkhtmltopdf/static-build/centos
./build.sh
popd

wkhtmltopdf/static-build/centos/qt/bin/qmake ichabod.pro
make

pushd wkhtmltopdf/static-build/centos/qt_build
make clean
popd

./test.sh 

./archive_src.sh

./build_rpm.sh
