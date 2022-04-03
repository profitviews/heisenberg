#[=======================================================================[.rst:
clang_format_target
------------------

Overview
^^^^^^^^

Generates a target for running the clang-format executable on all of the source 
files in a target.

.. code-block:: cmake

  clang_format_target(
      [TARGET <target name>]
  )
   -- Call the specified command for the version of an exchange for specifed
      target type. 

  ``TARGET`` specify the target containing the source files to run clang-format
    on.

#]=======================================================================]
function(clant_format_target)
    set(options QUIET)
    set(oneValueArgs TARGET CONFIG_FILE)
    set(multiValueArgs)
    cmake_parse_arguments(CLANG_FORMAT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )
    
    find_program(CLANG_FORMAT_PATH clang-format)
    if (NOT CLANG_FORMAT_PATH)
        message(WARNING "clang-format not found, disabling support.")
        return()
    endif()

    execute_process(COMMAND "${CLANG_FORMAT_PATH}" --version OUTPUT_VARIABLE CLANG_FORMAT_OUTPUT)
    string(REPLACE "\n" ";" CLANG_FORMAT_OUTPUT "${CLANG_FORMAT_OUTPUT}")
    foreach(line ${CLANG_FORMAT_OUTPUT})
        string(REGEX REPLACE "clang-format version ([\\.0-9]+)$" "\\1" CLANG_FORMAT_VERSION "${line}")
        if(CLANG_FORMAT_VERSION)
            break()
        endif()
    endforeach()

    if("${CLANG_FORMAT_VERSION}" VERSION_LESS 13.0.0)
        message(STATUS "Unsupported version of Clang-Format found.  Version 13.0.0 requird, version ${CLANG_FORMAT_VERSION} found")
    endif()

    get_property(CLANG_FORMAT_QUIET_REPORTED GLOBAL PROPERTY clang_format_quiet_reported)
    if(NOT CLANG_FORMAT_QUIET AND NOT CLANG_FORMAT_QUIET_REPORTED)
        set_property(GLOBAL PROPERTY clang_format_quiet_reported TRUE)
        message(STATUS "Enabling Clang-Format support.  Version: ${CLANG_FORMAT_VERSION}")
    endif()

    if(CLANG_FORMAT_TARGET)
        get_target_property(targetSources ${CLANG_FORMAT_TARGET} SOURCES)
        if (targetSources)
            list(APPEND targetAllSources ${targetSources})
        endif()
        get_target_property(targetInferfaceSources ${CLANG_FORMAT_TARGET} INTERFACE_SOURCES)
        if (targetInferfaceSources)
            list(APPEND targetAllSources ${targetInferfaceSources})
        endif()
        list(REMOVE_DUPLICATES targetAllSources)
        foreach(clangFormatSource ${targetAllSources})
            get_filename_component(clangFormatSource ${clangFormatSource} ABSOLUTE)
            list(APPEND clangFormatSources ${clangFormatSource})
        endforeach()
    endif()

    add_custom_target(clangformat_${CLANG_FORMAT_TARGET}
        COMMAND
            ${CLANG_FORMAT_PATH} -style=file -i ${clangFormatSources}
        WORKING_DIRECTORY
            ${CMAKE_SOURCE_DIR}
        COMMENT
            "Formatting ${CLANG_FORMAT_TARGET} files with ${CLANG_FORMAT_EXE}"
    )

    if(TARGET clangformat)
        add_dependencies(clangformat clangformat_${CLANG_FORMAT_TARGET})
    else()
        add_custom_target(clangformat DEPENDS clangformat_${CLANG_FORMAT_TARGET})
    endif()
endfunction()