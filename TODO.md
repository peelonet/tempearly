# TODO list

## Interpreter

- Builtin support for SQLite
- Interfaces/mixins for classes
- More detailed error messages. (Include source position.)
- Streams: Buffered input.
- Streams: Line number counting.

## API

- Regular expressions
- Database connectivity
- Date/time
- Streams
- File IO
- Filename globbing

## Server APIs

- AST caching in Apache and built-in HTTPD SAPIs.
- Process multipart HTTP requests.
- Parse request parameters (especially POSTed ones) only when requested in the
  script.
- Built-in HTTPD: Return `505 HTTP Version Not Supported` if client sends HTTP
  request with unrecognized HTTP version.

# Maybe list

- Multiple garbage collector implementations or customizable garbage collector. (Mainly
  for Apache2 SAPI so that the GC could use memory pools provided by Apache HTTPD.)
