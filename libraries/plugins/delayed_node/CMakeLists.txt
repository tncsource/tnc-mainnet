file(GLOB HEADERS "include/sigmaengine/delayed_node/*.hpp")

add_library( sigmaengine_delayed_node
             delayed_node_plugin.cpp
           )

target_link_libraries( sigmaengine_delayed_node sigmaengine_chain sigmaengine_protocol sigmaengine_app )
target_include_directories( sigmaengine_delayed_node
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(MSVC)
  set_source_files_properties( delayed_node_plugin.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

install( TARGETS
   sigmaengine_delayed_node

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
