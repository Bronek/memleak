# Add compilation options appropriate for the current compiler

function(append_compilation_options)
    set(options OPTIMIZATION WARNINGS)
    set(Options_NAME ${ARGV0})
    cmake_parse_arguments(Options "${options}" "" "" ${ARGN})

    if(NOT DEFINED Options_NAME)
        message(FATAL_ERROR "NAME must be set")
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        message(FATAL_ERROR "Windows is not supported at this time")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "(Apple)?Clang")
        if(Options_WARNINGS)
            target_compile_options(${Options_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror -Wno-redundant-move)
        endif()

        if(Options_OPTIMIZATION)
            target_compile_options(${Options_NAME} PRIVATE $<IF:$<CONFIG:Debug>,-O0 -fno-omit-frame-pointer,-O2>)
        endif()
    endif()
endfunction()
