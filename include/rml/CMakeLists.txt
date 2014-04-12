include (${rml_cmake_dir}/RMLUtils.cmake)

set (headers
  RMLHelper.hh
)

foreach (hdr ${headers})
  APPEND_TO_CACHED_STRING(rml_headers
    "SDF Headers" "#include <rml/${hdr}>\n")
endforeach()
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/rml.hh.in
                ${CMAKE_CURRENT_BINARY_DIR}/rml.hh)

rml_install_includes("" ${headers} ${CMAKE_CURRENT_BINARY_DIR}/rml.hh)
