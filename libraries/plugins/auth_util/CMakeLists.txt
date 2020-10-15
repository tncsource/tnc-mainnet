file(GLOB HEADERS "include/sigmaengine/plugins/auth_util/*.hpp")

add_library( sigmaengine_auth_util
             ${HEADERS}
             auth_util_plugin.cpp
             auth_util_api.cpp
           )

target_link_libraries( sigmaengine_auth_util sigmaengine_app sigmaengine_chain sigmaengine_protocol fc )
target_include_directories( sigmaengine_auth_util
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
