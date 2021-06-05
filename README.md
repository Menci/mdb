# Menci DeBugger

Simple Linux x86_64 debugger with ptrace(2).

Only works with single-threaded static-linked programs.

# Dependencies

* Linux
* C++ compiler with C++ 17 support
* CMake (>= 3.0)
* GNU Readline
* GNU Source-highlight
* Zydis


# Build

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j
```

# Usage

```bash
# In build/ directory
bin/mdb ~/a.out # The being-debugged file
```

Enter `h` for help.
