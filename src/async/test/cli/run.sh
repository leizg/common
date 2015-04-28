#!/bin/bash

is_build=0
[ -n "$1" ] && { is_build=1 && shift; }

build_command='scons -j4'

bin='../../../build/cli'
args='-alsologtostderr -client_number=1 -count=1'

if [ $is_build -eq 1 ] 
then
  $build_command
  if [ $? -ne 0 ]  
  then
    echo "build error"
    exit -1
  fi
fi

#env HEAPCHECK=normal PPROF_PATH=/usr/local/sbin/pprof
gdb --ex "handle SIGPIPE nostop noprint" --ex run --args \
  $bin $args

exit 0
