## Static Analyzer

ref: https://github.com/TheLartians/ModernCppStarter

### include-what-you-use

install the version adapted to your clang version: https://github.com/include-what-you-use/include-what-you-use

```sh
mkdir build && cd build
cmake -DUSE_STATIC_ANALYZER=iwyu ..
make -j8
```

### cppcheck

```sh
sudo apt install cppcheck
cppcheck --language=c++ --std=c++14 --enable=all src include
```

### clang-tidy

install the version adapted to your clang version, such as: sudo apt install clang-tidy-9

```sh
cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang ..
make -j8
```

modify the rules in .clang-tidy file.

## Unittest

ref: https://zhuanlan.zhihu.com/p/369466622

```sh
mkdir build && cd build
cmake ..
make -j8
./demo_unittest
```

## Code Coverage

ref: https://blog.csdn.net/yjj546542806/article/details/111568642

```sh
mkdir build && cd build
cmake -DENABLE_COVERAGE=1 ..
make -j8
make demo_coverage
genhtml -o result demo_coverage.info
```

and result/index.html will show you the code coverage.

## Format Code

```sh
clang-format -verbose -i src/* include/*
```

modify the rules in .clang-format file.
caution: run clang-format with '-i' option will fix the file directly.

## Memory Profiler

### AddressSanitizer

ref: https://github.com/google/sanitizers

```sh
mkdir build && cd build
cmake -DDEBUG=1 -DUSE_SANITIZER=Address ..
make -j8
./test_demo
```

### tcmalloc in gperftools

ref:
  http://qiushao.net/2020/07/11/Linux/memory-leak-analyze-tcmalloc/
  https://github.com/gperftools/gperftools

```sh
mkdir build && cd build
cmake -DDEBUG=1 ..
make -j8
export PPROF_PATH=/usr/local/bin/pprof
LD_PRELOAD=/usr/local/lib/libtcmalloc.so HEAPCHECK=normal ./test_demo
```

or:

```sh
mkdir build && cd build
cmake -DDEBUG=1 ..
make -j8
export PPROF_PATH=/usr/local/bin/pprof
LD_PRELOAD=/usr/local/lib/libtcmalloc.so HEAPPROFILE=./test.hprof ./test_demo
pprof --text --show_bytes ./test_demo ./test.hprof.0001.heap
```

## CPU profiler

ref:
  https://blog.csdn.net/10km/article/details/83820080
  https://gperftools.github.io/gperftools/cpuprofile.html

```sh
mkdir build && cd build
cmake ..
make -j8
export PPROF_PATH=/usr/local/bin/pprof
LD_PRELOAD=/usr/local/lib/libprofiler.so CPUPROFILE=test_demo.prof ./test_demo
pprof --text ./test_demo test_demo.prof
```

## Cross Compile

Such as:

```sh
cmake -DARCH=aarch64 ..
make -j8
```

## Debug

### Crashpad

ref: 
  https://chromium.googlesource.com/crashpad/crashpad
  https://chromium.googlesource.com/breakpad/breakpad

test_demo is linked with crashpad, when test_demo crashed, for example, function crash() as follows is called,

```c++
void crash() { volatile int* a = nullptr; *a = 1; }
```

then, a "reports" folder will be generated, find the .dmp file generated in "reports/pending/", such as "bdbcf254-9cea-4c1c-a531-e749b9db2fdf.dmp", do as follows:

```sh
dump_syms test_demo > test_demo.sym
mkdir -p ./symbols/`head -n1 test_demo.sym | awk '{print $5}'`/`head -n1 test_demo.sym | awk '{print $4}'`
mv test_demo.sym ./symbols/`head -n1 test_demo.sym | awk '{print $5}'`/`head -n1 test_demo.sym | awk '{print $4}'`/

dump_syms libdemo.so > libdemo.so.sym
mkdir -p ./symbols/`head -n1 libdemo.so.sym | awk '{print $5}'`/`head -n1 libdemo.so.sym | awk '{print $4}'`
mv libdemo.so.sym ./symbols/`head -n1 libdemo.so.sym | awk '{print $5}'`/`head -n1 libdemo.so.sym | awk '{print $4}'`/

minidump_stackwalk reports/pending/bdbcf254-9cea-4c1c-a531-e749b9db2fdf.dmp ./symbols
```sh

then minidump_stackwalk will show you which file and the line number the code crashed at.
PS: 
a. when running dump_syms, the ELF file must have debug symbols.
b. dump_syms and minidump_stackwalk are tools in breakpad, binary files built with Linux x86_64 Ubuntu18.04 are in ../Common/third_party/crashpad
