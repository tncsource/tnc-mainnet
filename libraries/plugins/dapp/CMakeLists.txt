file( GLOB HEADERS
      "include/sigmaengine/dapp/*.hpp" )

add_library( sigmaengine_dapp
             ${HEADERS}
             dapp_api.cpp
             dapp_plugin.cpp
             dapp_evaluators.cpp
             dapp_operations.cpp
           )

target_link_libraries( sigmaengine_dapp sigmaengine_app sigmaengine_chain fc sigmaengine_protocol )
target_include_directories( sigmaengine_dapp
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   sigmaengine_dapp
   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
