FIND_PROGRAM(
    APXS_BIN
    NAMES apxs2 apxs
)

IF (APXS_BIN)
    EXECUTE_PROCESS(
        COMMAND ${APXS_BIN} -q INCLUDEDIR
        OUTPUT_VARIABLE APXS2_INCLUDEDIRS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    EXECUTE_PROCESS(
        COMMAND ${APXS_BIN} -q CFLAGS
        OUTPUT_VARIABLE APXS2_C_FLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    EXECUTE_PROCESS(
        COMMAND ${APXS_BIN} -q LDFLAGS
        OUTPUT_VARIABLE APXS2_LIBRARIES
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    EXECUTE_PROCESS(
        COMMAND ${APXS_BIN} -q libexecdir
        OUTPUT_VARIABLE MOD_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    SET(
        APACHE_MODULE_DIR
        "${MOD_DIR}"
        CACHE PATH
        "Installation directory for Apache modules"
    )
ELSE()
    MESSAGE(FATAL_ERROR "Could not find apxs")
ENDIF()
