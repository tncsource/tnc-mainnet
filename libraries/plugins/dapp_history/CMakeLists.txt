file(GLOB HEADERS "include/sigmaengine/dapp_history/*.hpp")

add_library( sigmaengine_dapp_history
               ${HEADERS}
               dapp_history_api.cpp
               dapp_history_plugin.cpp
               dapp_impacted.cpp
           )

target_link_libraries( sigmaengine_dapp_history 
                        sigmaengine_chain sigmaengine_protocol sigmaengine_app 
                        sigmaengine_dapp sigmaengine_token )
target_include_directories( sigmaengine_dapp_history
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   sigmaengine_dapp_history

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
