# TODO list

## Interpreter

- Abstract parser class
- Streams which can only read text content
- Builtin support for SQLite
- Interfaces/mixins for classes
- More detailed error messages. (Include source position.)

## API

- Regular expressions
- Database connectivity
- Date/time
- Streams
- File IO
- Filename globbing

## Server APIs

- AST caching in Apache module.
- Process multipart HTTP requests.
- FastCGI: AST caching which watches for modifications in the script similiar
  to what's found in built-in HTTPD.

# Maybe list

- Multiple garbage collector implementations or customizable garbage collector. (Mainly
  for Apache2 SAPI so that the GC could use memory pools provided by Apache HTTPD.)
