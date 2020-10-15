file(GLOB HEADERS "include/sigmaengine/plugins/debug_node/*.hpp")

add_library( sigmaengine_debug_node
             ${HEADERS}
             debug_node_plugin.cpp
             debug_node_api.cpp
           )

target_link_libraries( sigmaengine_debug_node sigmaengine_app sigmaengine_chain sigmaengine_protocol fc )
target_include_directories( sigmaengine_debug_node
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS sigmaengine_debug_node
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib
       )
INSTALL( FILES ${HEADERS} DESTINATION "include/sigmaengine/plugins/debug_node" )
