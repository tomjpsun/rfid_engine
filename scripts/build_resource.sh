#!/bin/bash

# we are in scritps directory
dir=$PWD

# root dir is in upper layer
root_dir="$(dirname "$dir")"

. $root_dir/version.mk
ARCH=`uname -m`
echo "build resource for install-${MAJOR}.${MINOR}.${SUB_MINOR}-${ARCH}.sh"


cd $root_dir
make
make install


cd $dir
mkdir -p res
cp $root_dir/librfidmgr.so.$MAJOR.$MINOR res/
cp $root_dir/config.json res/
mkdir -p res/inc
cp -r $root_dir/inc/* res/inc
cp $root_dir/version.mk res


ls res/
tar cjvf res-${MAJOR}.${MINOR}.${SUB_MINOR}-${ARCH}.tar.bz2 res/
