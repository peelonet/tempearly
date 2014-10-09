FIND_PATH(
    FASTCGI_INCLUDE_DIR
    fcgio.h
    PATHS
    /usr/include
    /usr/local/include
    /usr/include/fastcgi
    "$ENV{LIB_DIR}/include"
    "$ENV{INCLUDE}"
)

FIND_LIBRARY(
    FASTCGI_LIBRARY
    NAMES
    fcgi libfcgi
    PATHS
    /usr/local/lib
    /usr/lib
    "$ENV{LIB_DIR}/lib"
    "$ENV{LIB}"
)

IF (FASTCGI_INCLUDE_DIR AND FASTCGI_LIBRARY)
    SET(FASTCGI_FOUND TRUE)
ENDIF()

IF (FASTCGI_FOUND)
    MESSAGE(STATUS "Found: FastCGI: ${FASTCGI_LIBRARY}")
ELSE()
    MESSAGE(FATAL_ERROR "Could not find FastCGI")
ENDIF()
