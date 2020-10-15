file(GLOB HEADERS "include/sigmaengine/account_by_key/*.hpp")

add_library( sigmaengine_account_by_key
             account_by_key_plugin.cpp
             account_by_key_api.cpp
           )

target_link_libraries( sigmaengine_account_by_key sigmaengine_chain sigmaengine_protocol sigmaengine_app )
target_include_directories( sigmaengine_account_by_key
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   sigmaengine_account_by_key

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
