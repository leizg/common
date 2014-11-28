#!/bin/bash

set -e

cmd='apt-get install -y ' 

libs="libgoogle-glog-dev libgflags-dev \
 libprotobuf-dev libgtest-dev libgoogle-perftools-dev"

bin='scons build-essential gdb'

apt-get update 
$cmd $libs
$cmd $bin

set +e

exit 0
