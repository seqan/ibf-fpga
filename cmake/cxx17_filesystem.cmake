
include(CheckCXXSymbolExists)
check_cxx_symbol_exists(std::filesystem::file_size filesystem HAVE_CXX17_FILESYSTEM)

if (NOT HAVE_CXX17_FILESYSTEM)
    message(AUTHOR_WARNING "Use seem to use a compiler which does not automatically link to #include <filesystem> library. We'll try `-lstdc++fs`
.")
    link_libraries("stdc++fs")
endif ()
