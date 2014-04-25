#!/bin/bash

yum install -y python-multiprocessing gcc44-c++ libX11-devel libXext-devel libXrender-devel
yum install -y freetype-devel fontconfig-devel
yum install -y xorg-x11-xfs xorg-x11-xfs-utils  rpm-build

ln -s /usr/bin/g++44 /usr/bin/g++

git submodule add https://github.com/cesanta/mongoose.git
git submodule add https://github.com/wkhtmltopdf/wkhtmltopdf.git
git submodule update --init

pushd wkhtmltopdf
git submodule update --init
cd qt
git checkout wk_4.8
popd

./gen.py

pushd wkhtmltopdf/static-build/centos
./build.sh
popd

wkhtmltopdf/static-build/centos/qt/bin/qmake ichabod.pro
make
