#!/bin/bash

yum install -y python-multiprocessing gcc44-c++ libX11-devel libXext-devel libXrender-devel
yum install -y freetype-devel fontconfig-devel
yum install -y xorg-x11-xfs xorg-x11-xfs-utils  rpm-build

ln -s /usr/bin/g++44 /usr/bin/g++

function subm() {
    git submodule add https://github.com/cesanta/mongoose.git
    git submodule add https://github.com/wkhtmltopdf/wkhtmltopdf.git
    git submodule update --init

    pushd wkhtmltopdf
    git submodule update --init
    cd qt
    git checkout wk_4.8
    popd
}

VERSION=`cat version`

./gen.py

pushd wkhtmltopdf/static-build/centos
./build.sh
popd

wkhtmltopdf/static-build/centos/qt/bin/qmake ichabod.pro
make

mkdir srcbuild
pushd srcbuild
git clone https://github.com/moseymosey/ichabod.git
pushd ichabod
subm
popd
echo "Tarring SRPM..."
tar cf ichabod-$VERSION.tar ichabod/
echo "Zipping SRPM..."
gzip ichabod-$VERSION.tar
mv ichabod-$VERSION.tar.gz ../
popd
echo "Cleaning up..."
rm -rf srcbuild


name='ichabod'
name=${name%.spec}
topdir=$(mktemp -d)
version=$(cat version)
builddir=${TMPDIR:-/tmp}/${name}-${version}
sourcedir="${topdir}/SOURCES"
buildroot="${topdir}/BUILD/${name}-${version}-root"
mkdir -p ${topdir}/RPMS ${topdir}/SRPMS ${topdir}/SOURCES ${topdir}/BUILD
mkdir -p ${buildroot} ${builddir}
echo "=> Copying sources..."
mv ichabod-$VERSION.tar.gz ${topdir}/SOURCES
echo "=> Building RPM..."
rpmbuild --define "_topdir ${topdir}" --buildroot ${buildroot} --clean -ba ${name}.spec
echo ${topdir}
