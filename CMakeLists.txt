CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(tempearly C CXX)

INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFileCXX)

CHECK_INCLUDE_FILE_CXX(cstdint TEMPEARLY_HAVE_CSTDINT)
IF (NOT ${TEMPEARLY_HAVE_CSTDINT})
    CHECK_INCLUDE_FILE(stdint.h TEMPEARLY_HAVE_STDINT_H)
    IF (NOT ${TEMPEARLY_HAVE_STDINT_H})
        CHECK_INCLUDE_FILE_CXX(cinttypes TEMPEARLY_HAVE_CINTTYPES)
        IF (NOT ${TEMPEARLY_HAVE_CINTTYPES})
            CHECK_INCLUDE_FILE(inttypes.h TEMPEARLY_HAVE_INTTYPES_H)
        ENDIF()
    ENDIF()
ENDIF()

CHECK_INCLUDE_FILE_CXX(climits TEMPEARLY_HAVE_CLIMITS)
IF (NOT ${TEMPEARLY_HAVE_CLIMITS})
    CHECK_INCLUDE_FILE(limits.h TEMPEARLY_HAVE_LIMITS_H)
ENDIF()

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h
)

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
    src/utils.cc
    src/value.cc
    src/api/bool.cc
    src/api/iterator.cc
    src/api/list.cc
    src/api/number.cc
    src/api/object.cc
    src/api/string.cc
    src/api/void.cc
)

ADD_EXECUTABLE(
    tempearly-cgi
    src/sapi-cgi.cc
    ${INTERPRETER_SOURCES}
)
