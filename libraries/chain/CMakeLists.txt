file(GLOB HEADERS "include/sigmaengine/chain/*.hpp" "include/sigmaengine/chain/util/*.hpp")

## SORT .cpp by most likely to change / break compile
add_library( sigmaengine_chain

             # As database takes the longest to compile, start it first
             database.cpp
             fork_database.cpp
             bobserver_schedule.cpp

             sigmaengine_evaluator.cpp

             sigmaengine_objects.cpp
             shared_authority.cpp
             block_log.cpp

             util/reward.cpp

             ${HEADERS}
           )

target_link_libraries( sigmaengine_chain sigmaengine_protocol fc chainbase graphene_schema ${PATCH_MERGE_LIB} )
target_include_directories( sigmaengine_chain
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" "${CMAKE_CURRENT_BINARY_DIR}/include" )

if(MSVC)
  set_source_files_properties( database.cpp PROPERTIES COMPILE_FLAGS "/bigobj" )
endif(MSVC)

INSTALL( TARGETS
   sigmaengine_chain

   RUNTIME DESTINATION bin
   LIBRARY DESTINATION lib
   ARCHIVE DESTINATION lib
)
INSTALL( FILES ${HEADERS} DESTINATION "include/sigmaengine/chain" )
