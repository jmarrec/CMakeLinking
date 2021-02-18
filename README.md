# CMake Linking - A test framework

A simple project to test how CMake handles linking.

The idea is to mess with PUBLIC/PRIVATE linking, and STATIC versus SHARED, and look at what the resulting executable actually links to.

```bash
mkdir build
cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
```

```bash
ldd Products/Test
    linux-vdso.so.1 (0x00007ffdcdbf9000)
    libprintname.so => /home/julien/Software/CMakeLinking/build/Products/libprintname.so (0x00007f9d85940000)
    libstdc++.so.6 => /usr/lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007f9d8556c000)
    libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007f9d85354000)
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f9d84f63000)
    libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f9d84bc5000)
    /lib64/ld-linux-x86-64.so.2 (0x00007f9d85d45000)
```
