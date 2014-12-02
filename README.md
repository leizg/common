## dreamio
===
library about network programing(c++ implement).

run test:
```
  step1: start echo server
      cd src/io/test/serv
      ./run.sh build

  step2: start echo client
      cd src/io/test/cli
      ./run.sh build

```

## depends:
```
  glog 
  gflags
  protobuf
  google-perftools

install libraries under ubuntu:
  script/install_depend.sh
```

### platform:
  support linux and mac.
