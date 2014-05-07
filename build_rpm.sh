#!/bin/bash

name='ichabod'
name=${name%.spec}
topdir=$(mktemp -d)
version=$(cat version.h | awk '{print $3;}' | tr -d \")
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
