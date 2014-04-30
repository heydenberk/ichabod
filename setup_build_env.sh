#!/bin/bash

yum install -y python-multiprocessing gcc44-c++ libX11-devel libXext-devel libXrender-devel
yum install -y freetype-devel fontconfig-devel
yum install -y xorg-x11-xfs xorg-x11-xfs-utils  rpm-build
yum install -y libtool.i386 automake autoconf m4

ln -s /usr/bin/g++44 /usr/bin/g++

