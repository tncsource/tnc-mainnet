#include <sigmaengine/protocol/version.hpp>
#include <sigmaengine/chain/sigmaengine_object_types.hpp>

namespace sigmaengine { namespace chain {

class hardfork_property_object : public object< hardfork_property_object_type, hardfork_property_object >
{
   public:
      template< typename Constructor, typename Allocator >
      hardfork_property_object( Constructor&& c, allocator< Allocator > a )
         :processed_hardforks( a.get_segment_manager() )
      {
         c( *this );
      }

      id_type                                                              id;

      bip::vector< fc::time_point_sec, allocator< fc::time_point_sec > >   processed_hardforks;
      uint32_t                                                             last_hardfork = 0;
      protocol::hardfork_version                                           current_hardfork_version;
      protocol::hardfork_version                                           next_hardfork;
      fc::time_point_sec                                                   next_hardfork_time;
};

typedef multi_index_container<
   hardfork_property_object,
   indexed_by<
      ordered_unique< member< hardfork_property_object, hardfork_property_object::id_type, &hardfork_property_object::id > >
   >,
   allocator< hardfork_property_object >
> hardfork_property_index;

} } // namespace sigmaengine::chain

FC_REFLECT( sigmaengine::chain::hardfork_property_object,
   (id)(processed_hardforks)(last_hardfork)(current_hardfork_version)
   (next_hardfork)(next_hardfork_time) )
CHAINBASE_SET_INDEX_TYPE( sigmaengine::chain::hardfork_property_object, sigmaengine::chain::hardfork_property_index )
