CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(tempearly C CXX)

OPTION(
    TEMPEARLY_ENABLE_CGI_SAPI
    "Enable CGI SAPI"
    ON
)

OPTION(
    TEMPEARLY_ENABLE_FASTCGI_SAPI
    "Enable FastCGI SAPI"
    OFF
)

OPTION(
    TEMPEARLY_ENABLE_HTTPD_SAPI
    "Enable builtin HTTP server SAPI"
    OFF
)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

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

CHECK_INCLUDE_FILE_CXX(cfloat TEMPEARLY_HAVE_CFLOAT)
IF (NOT ${TEMPEARLY_HAVE_CFLOAT})
    CHECK_INCLUDE_FILE(float.h TEMPEARLY_HAVE_FLOAT_H)
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
    src/coreobject.cc
    src/interpreter.cc
    src/memory.cc
    src/node.cc
    src/object.cc
    src/parser.cc
    src/result.cc
    src/scope.cc
    src/script.cc
    src/token.cc
    src/utils.cc
    src/value.cc
    src/api/bool.cc
    src/api/class.cc
    src/api/exception.cc
    src/api/function.cc
    src/api/iterator.cc
    src/api/list.cc
    src/api/number.cc
    src/api/object.cc
    src/api/request.cc
    src/api/response.cc
    src/api/string.cc
    src/api/void.cc
    src/core/bytestring.cc
    src/core/date.cc
    src/core/datetime.cc
    src/core/filename.cc
    src/core/random.cc
    src/core/string.cc
    src/core/stringbuilder.cc
    src/core/time.cc
    src/http/cookie.cc
    src/http/method.cc
    src/http/version.cc
    src/net/socket.cc
    src/sapi/request.cc
    src/sapi/response.cc
)

IF (TEMPEARLY_ENABLE_CGI_SAPI)
    ADD_EXECUTABLE(
        tempearly-cgi
        src/sapi/cgi/cgi-request.cc
        src/sapi/cgi/cgi-response.cc
        src/sapi/cgi/main.cc
        ${INTERPRETER_SOURCES}
    )
ENDIF()

IF (TEMPEARLY_ENABLE_FASTCGI_SAPI)
    INCLUDE(FindFastCGI)
    ADD_EXECUTABLE(
        tempearly-fcgi
        src/sapi/cgi/cgi-request.cc
        src/sapi/cgi/cgi-response.cc
        src/sapi/fastcgi/main.cc
        ${INTERPRETER_SOURCES}
    )
    TARGET_LINK_LIBRARIES(
        tempearly-fcgi
        ${FASTCGI_LIBRARY}
    )
ENDIF()

IF (TEMPEARLY_ENABLE_HTTPD_SAPI)
    ADD_EXECUTABLE(
        tempearly-httpd
        src/sapi/httpd/httpd-request.cc
        src/sapi/httpd/httpd-response.cc
        src/sapi/httpd/main.cc
        src/sapi/httpd/server.cc
        ${INTERPRETER_SOURCES}
    )
ENDIF()
