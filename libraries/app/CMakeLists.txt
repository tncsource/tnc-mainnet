file(GLOB HEADERS "include/sigmaengine/app/*.hpp")

add_library( sigmaengine_app
             database_api.cpp
             api.cpp
             application.cpp
             impacted.cpp
             plugin.cpp
             ${HEADERS}
           )

target_link_libraries( sigmaengine_app sigmaengine_chain sigmaengine_protocol 
                     sigmaengine_bobserver sigmaengine_mf_plugins fc graphene_net 
                     sigmaengine_token sigmaengine_dapp sigmaengine_dapp_history 
                     graphene_utilities 
                     )
target_include_directories( sigmaengine_app
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )

if(MSVC)
  set_source_files_properties( application.cpp api.cpp database_api.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

INSTALL( TARGETS
   sigmaengine_app

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/sigmaengine/app" )
