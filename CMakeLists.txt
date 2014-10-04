CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(tempearly C CXX)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/src)

SET(
    INTERPRETER_SOURCES
    src/class.cc
    src/cookie.cc
    src/coreobject.cc
    src/functionobject.cc
    src/interpreter.cc
    src/memory.cc
    src/node.cc
    src/object.cc
    src/parser.cc
    src/request.cc
    src/response.cc
    src/result.cc
    src/scope.cc
    src/script.cc
    src/token.cc
    src/value.cc
    src/api/bool.cc
    src/api/object.cc
)

ADD_EXECUTABLE(
    tempearly-cgi
    src/sapi-cgi.cc
    ${INTERPRETER_SOURCES}
)
