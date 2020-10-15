file(GLOB HEADERS "include/sigmaengine/bobserver/*.hpp")

add_library( sigmaengine_bobserver
             bobserver_plugin.cpp
             bobserver_evaluators.cpp
             bobserver_operations.cpp
           )

target_link_libraries( sigmaengine_bobserver sigmaengine_chain sigmaengine_protocol sigmaengine_app )
target_include_directories( sigmaengine_bobserver
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

install( TARGETS
   sigmaengine_bobserver

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
