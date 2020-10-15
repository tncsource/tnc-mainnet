file(GLOB HEADERS "include/sigmaengine/txt_test_gen/*.hpp")

add_library( sigmaengine_txt_test_gen
             txt_test_gen_plugin.cpp
             txt_test_gen_api.cpp
           )

target_link_libraries( sigmaengine_txt_test_gen sigmaengine_chain sigmaengine_protocol sigmaengine_app )
target_include_directories( sigmaengine_txt_test_gen
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(MSVC)
  set_source_files_properties( delayed_node_plugin.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

install( TARGETS
   sigmaengine_txt_test_gen

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
