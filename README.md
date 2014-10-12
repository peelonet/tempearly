Tempearly
=========

Minimal scripting language / templating engine which is meant to be replacement
for PHP.

Meant for small scripts/hacks/CGI-applications, not for large web sites.

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
