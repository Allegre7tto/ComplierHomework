#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "Compiler::lexer" for configuration "Debug"
set_property(TARGET Compiler::lexer APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Compiler::lexer PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/liblexer.a"
  )

list(APPEND _cmake_import_check_targets Compiler::lexer )
list(APPEND _cmake_import_check_files_for_Compiler::lexer "${_IMPORT_PREFIX}/lib/liblexer.a" )

# Import target "Compiler::parser" for configuration "Debug"
set_property(TARGET Compiler::parser APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Compiler::parser PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libparser.a"
  )

list(APPEND _cmake_import_check_targets Compiler::parser )
list(APPEND _cmake_import_check_files_for_Compiler::parser "${_IMPORT_PREFIX}/lib/libparser.a" )

# Import target "Compiler::semantic" for configuration "Debug"
set_property(TARGET Compiler::semantic APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(Compiler::semantic PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libsemantic.a"
  )

list(APPEND _cmake_import_check_targets Compiler::semantic )
list(APPEND _cmake_import_check_files_for_Compiler::semantic "${_IMPORT_PREFIX}/lib/libsemantic.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
