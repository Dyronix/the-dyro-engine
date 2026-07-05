# -------------------------------
# Optional "docs" target that regenerates the html documentation in docs/html
# from the engine headers and the guide pages. Only appears when doxygen is
# installed; the generated html is committed, so nobody needs doxygen just to
# read the docs.
# -------------------------------
FIND_PACKAGE(Doxygen QUIET)
IF(DOXYGEN_FOUND)
    ADD_CUSTOM_TARGET(docs
        COMMAND Doxygen::doxygen Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs
        COMMENT "Generating documentation into docs/html"
        VERBATIM)
    SET_PROPERTY(TARGET docs PROPERTY FOLDER "events")
ENDIF()
