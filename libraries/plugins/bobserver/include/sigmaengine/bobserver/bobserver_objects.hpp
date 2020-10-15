#pragma once

#include <sigmaengine/app/plugin.hpp>
#include <sigmaengine/chain/sigmaengine_object_types.hpp>

#include <boost/multi_index/composite_key.hpp>

namespace sigmaengine { namespace bobserver {

using namespace std;
using namespace sigmaengine::chain;

enum bobserver_plugin_object_type
{
   account_bandwidth_object_type = ( BOBSERVER_SPACE_ID << 8 ),
   content_edit_lock_object_type = ( BOBSERVER_SPACE_ID << 8 ) + 1,
   reserve_ratio_object_type      = ( BOBSERVER_SPACE_ID << 8 ) + 2
};

enum bandwidth_type
{
   post    ///< Rate limiting posting reward eligibility over time
};

class content_edit_lock_object : public object< content_edit_lock_object_type, content_edit_lock_object >
{
   public:
      template< typename Constructor, typename Allocator >
      content_edit_lock_object( Constructor&& c, allocator< Allocator > a )
      {
         c( *this );
      }

      content_edit_lock_object() {}

      id_type           id;
      account_name_type account;
      time_point_sec    lock_time;
};

typedef oid< content_edit_lock_object > content_edit_lock_id_type;


class reserve_ratio_object : public object< reserve_ratio_object_type, reserve_ratio_object >
{
   public:
      template< typename Constructor, typename Allocator >
      reserve_ratio_object( Constructor&& c, allocator< Allocator > a )
      {
         c( *this );
      }

      reserve_ratio_object() {}

      id_type           id;

      /**
       *  Average block size is updated every block to be:
       *
       *     average_block_size = (99 * average_block_size + new_block_size) / 100
       *
       *  This property is used to update the current_reserve_ratio to maintain approximately
       *  50% or less utilization of network capacity.
       */
      int32_t    average_block_size = 0;

      /**
       *   Any time average_block_size <= 50% maximum_block_size this value grows by 1 until it
       *   reaches SIGMAENGINE_MAX_RESERVE_RATIO.  Any time average_block_size is greater than
       *   50% it falls by 1%.  Upward adjustments happen once per round, downward adjustments
       *   happen every block.
       */
      int64_t    current_reserve_ratio = 1;
};

typedef oid< reserve_ratio_object > reserve_ratio_id_type;

struct by_account;

typedef multi_index_container <
   content_edit_lock_object,
   indexed_by <
      ordered_unique< tag< by_id >,
         member< content_edit_lock_object, content_edit_lock_id_type, &content_edit_lock_object::id > >,
      ordered_unique< tag< by_account >,
         member< content_edit_lock_object, account_name_type, &content_edit_lock_object::account > >
   >,
   allocator< content_edit_lock_object >
> content_edit_lock_index;

typedef multi_index_container <
   reserve_ratio_object,
   indexed_by <
      ordered_unique< tag< by_id >,
         member< reserve_ratio_object, reserve_ratio_id_type, &reserve_ratio_object::id > >
   >,
   allocator< reserve_ratio_object >
> reserve_ratio_index;

} } // sigmaengine::bobserver

FC_REFLECT( sigmaengine::bobserver::content_edit_lock_object,
            (id)(account)(lock_time) )
CHAINBASE_SET_INDEX_TYPE( sigmaengine::bobserver::content_edit_lock_object, sigmaengine::bobserver::content_edit_lock_index )

FC_REFLECT( sigmaengine::bobserver::reserve_ratio_object,
            (id)(average_block_size)(current_reserve_ratio) )
CHAINBASE_SET_INDEX_TYPE( sigmaengine::bobserver::reserve_ratio_object, sigmaengine::bobserver::reserve_ratio_index )
