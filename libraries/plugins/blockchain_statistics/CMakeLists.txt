file(GLOB HEADERS "include/sigmaengine/blockchain_statistics/*.hpp")

add_library( sigmaengine_blockchain_statistics
             blockchain_statistics_plugin.cpp
             blockchain_statistics_api.cpp
           )

target_link_libraries( sigmaengine_blockchain_statistics sigmaengine_chain sigmaengine_protocol sigmaengine_app )
target_include_directories( sigmaengine_blockchain_statistics
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   sigmaengine_blockchain_statistics

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)