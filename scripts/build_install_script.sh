#!/bin/bash

# dir = scripts directory
dir=$PWD
# rootdir = parent directory
root_dir="$(dirname "$dir")"

. $root_dir/version.mk
ARCH=`uname -m`

# target install.sh
target=install-${MAJOR}.${MINOR}.${SUB_MINOR}-${ARCH}.sh

echo "generate install-${MAJOR}.${MINOR}.${SUB_MINOR}-${ARCH}.sh"

cat install.sh.in > ${target}
echo "PAYLOAD:" >> ${target}
cat res-${MAJOR}.${MINOR}.${SUB_MINOR}-${ARCH}.tar.bz2 >> ${target}
chmod +x ${target}
