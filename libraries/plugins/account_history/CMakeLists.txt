file(GLOB HEADERS "include/sigmaengine/account_history/*.hpp")

add_library( sigmaengine_account_history
             account_history_plugin.cpp
           )

target_link_libraries( sigmaengine_account_history sigmaengine_chain sigmaengine_protocol sigmaengine_app )
target_include_directories( sigmaengine_account_history
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   sigmaengine_account_history

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
