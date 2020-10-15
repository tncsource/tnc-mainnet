file(GLOB HEADERS "include/sigmaengine/token/*.hpp" "include/sigmaengine/token/util/*.hpp")

add_library( sigmaengine_token
             token_api.cpp
             token_evaluators.cpp
             token_operations.cpp
             token_plugin.cpp
           )

target_link_libraries( sigmaengine_token sigmaengine_chain sigmaengine_protocol sigmaengine_app sigmaengine_dapp )
target_include_directories( sigmaengine_token
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   sigmaengine_token

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
