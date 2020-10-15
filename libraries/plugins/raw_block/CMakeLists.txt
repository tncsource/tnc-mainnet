file(GLOB HEADERS "include/sigmaengine/plugins/raw_block/*.hpp")

add_library( sigmaengine_raw_block
             ${HEADERS}
             raw_block_plugin.cpp
             raw_block_api.cpp
           )

target_link_libraries( sigmaengine_raw_block sigmaengine_app sigmaengine_chain sigmaengine_protocol fc )
target_include_directories( sigmaengine_raw_block
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
