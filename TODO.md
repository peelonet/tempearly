# Interpreter

- Modify parser so that it doesn't require `Interpreter` instance. This enables us to compile
  and cache scripts before any requests are being made.
- Builtin support for SQLite
- Interfaces/mixins for classes

# API

- Regular expressions
- Database connectivity
- Date/time
- Streams
- File IO
- Filename globbing

# Server APIs

- AST caching in FastCGI and Apache SAPIs.
- Process multipart HTTP requests.
- Parse request parameters (especially POSTed ones) only when requested in the
  script.

# Maybe list

- Multiple garbage collector implementations or customizable garbage collector. (Mainly
  for Apache2 SAPI so that the GC could use memory pools provided by Apache HTTPD.)
