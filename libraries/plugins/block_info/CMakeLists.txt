file(GLOB HEADERS "include/sigmaengine/plugins/block_info/*.hpp")

add_library( sigmaengine_block_info
             ${HEADERS}
             block_info_plugin.cpp
             block_info_api.cpp
           )

target_link_libraries( sigmaengine_block_info sigmaengine_app sigmaengine_chain sigmaengine_protocol fc )
target_include_directories( sigmaengine_block_info
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
