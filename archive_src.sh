#!/bin/bash
source common.sh
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
