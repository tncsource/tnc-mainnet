add_executable( cli_wallet main.cpp )
if( UNIX AND NOT APPLE )
  set(rt_library rt )
endif()

find_package( Gperftools QUIET )
if( GPERFTOOLS_FOUND )
    message( STATUS "Found gperftools; compiling cli_wallet with TCMalloc")
    list( APPEND PLATFORM_SPECIFIC_LIBS tcmalloc )
endif()

if( SIGMAENGINE_STATIC_BUILD )
   # I don't know why sigmaengine_app is required twice in the following line, I just know the linker breaks if it isn't.
   target_link_libraries( cli_wallet PRIVATE
                          "-static-libstdc++ -static-libgcc -lreadline"
                          sigmaengine_app graphene_net sigmaengine_chain sigmaengine_protocol graphene_utilities 
                          sigmaengine_wallet sigmaengine_app fc sigmaengine_token 
                          sigmaengine_dapp sigmaengine_dapp_history ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
elseif( FULL_STATIC_BUILD )
target_link_libraries( cli_wallet PRIVATE
                          "-static-libstdc++ -static-libgcc -lreadline -Wl,-Bstatic -lstdc++ -lwinpthread -Wl,-Bdynamic"
                          sigmaengine_app graphene_net sigmaengine_chain sigmaengine_protocol graphene_utilities 
                          sigmaengine_wallet sigmaengine_app fc sigmaengine_token sigmaengine_dapp 
                          sigmaengine_dapp_history ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
else( SIGMAENGINE_STATIC_BUILD )
   target_link_libraries( cli_wallet PRIVATE
                          Threads::Threads
                          sigmaengine_app graphene_net sigmaengine_chain sigmaengine_protocol graphene_utilities 
                          sigmaengine_wallet sigmaengine_app fc sigmaengine_token sigmaengine_dapp 
                          sigmaengine_dapp_history ${CMAKE_DL_LIBS} ${PLATFORM_SPECIFIC_LIBS} )
endif( SIGMAENGINE_STATIC_BUILD )

if(MSVC)
  set_source_files_properties( main.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

install( TARGETS
   cli_wallet

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
