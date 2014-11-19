Tempearly
=========

Minimal scripting language / templating engine which aims to provide a sane
alternative for PHP.

It is meant for small scripts/hacks/demonstrations, it is not a framework
for building large web applications.

## Compilation

Requires [CMake](http://www.cmake.org) and C++-compiler.

```bash
mkdir build
cd build
cmake -DENABLE_HTTPD_SAPI=1 ..
make
./tempearly-httpd ../examples
```

After that, point your browser to `http://localhost:8000` and start running
some examples.
