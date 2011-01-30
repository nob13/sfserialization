# Main Library
file (GLOB_RECURSE src_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file (GLOB_RECURSE header_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)

add_library (sfserialization ${src_files} ${header_files})

install (FILES    ${header_files} DESTINATION include/sfserialization)
install (TARGETS  sfserialization ARCHIVE DESTINATION lib)
