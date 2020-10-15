
#include <sigmaengine/chain/database.hpp>
#include <sigmaengine/chain/bobserver_objects.hpp>
#include <sigmaengine/chain/bobserver_schedule.hpp>

#include <sigmaengine/protocol/config.hpp>

namespace sigmaengine { namespace chain {

void update_bobserver_schedule4( database& db )
{
   const bobserver_schedule_object& bo_schedule_object = db.get_bobserver_schedule_object();
   vector< account_name_type > active_bobservers;
   active_bobservers.reserve( SIGMAENGINE_NUM_BOBSERVERS );

   /// Add the highest voted bobservers
   flat_set< bobserver_id_type > selected_bp;
   selected_bp.reserve( bo_schedule_object.max_voted_bobservers );

   dlog( "BP : max_voted_bobservers = ${max BP}", ( "max BP", bo_schedule_object.max_voted_bobservers ) );

   const auto& bp_idx = db.get_index< bobserver_index >().indices().get< by_vote_name >();  
   for( auto itr = bp_idx.begin(); itr != bp_idx.end(); itr++ )
   {  // except a bo/bp in miner
      if ( itr->is_excepted && itr->signing_key != public_key_type() ) 
      {
         db.modify( *itr, [&]( bobserver_object& o ) { 
            o.signing_key = public_key_type();
         } );
         db.push_virtual_operation( shutdown_bobserver_operation( itr->account ) );
      }
   }

   for( auto itr = bp_idx.begin();
         itr != bp_idx.end() && selected_bp.size() < bo_schedule_object.max_voted_bobservers;
         ++itr )
   {
      if( itr->signing_key == public_key_type() )
         continue;

      selected_bp.insert( itr->id );
      active_bobservers.push_back( itr->account) ;
   }

   dlog( "BP : BP active = ${active}", ( "active", active_bobservers ) );

   auto num_bp = active_bobservers.size();

   flat_set< bobserver_id_type > selected_miners;
   selected_miners.reserve( bo_schedule_object.max_miner_bobservers );

   auto num_miners = selected_miners.size();
   auto num_timeshare = active_bobservers.size() - num_miners - num_bp;
   dlog( "BP : num_timeshare = ${num_time}, num_miners = ${num_miners}, num_bp = ${num_bp}"
      , ( "num_time", num_timeshare )( "num_miners", num_miners )( "num_bp", num_bp ) );

   /*********** check hardfork vote ***********/
   auto majority_version = bo_schedule_object.majority_version;

   flat_map< version, uint32_t, std::greater< version > > bobserver_versions;
   flat_map< std::tuple< hardfork_version, time_point_sec >, uint32_t > hardfork_version_votes;

   for( uint32_t i = 0; i < bo_schedule_object.num_scheduled_bobservers; i++ )
   {
      auto bobserver = db.get_bobserver( bo_schedule_object.current_shuffled_bobservers[ i ] );
      if( bobserver_versions.find( bobserver.running_version ) == bobserver_versions.end() )
         bobserver_versions[ bobserver.running_version ] = 1;
      else
         bobserver_versions[ bobserver.running_version ] += 1;

      auto version_vote = std::make_tuple( bobserver.hardfork_version_vote, bobserver.hardfork_time_vote );
      if( hardfork_version_votes.find( version_vote ) == hardfork_version_votes.end() )
         hardfork_version_votes[ version_vote ] = 1;
      else
         hardfork_version_votes[ version_vote ] += 1;
   }

   int bobservers_on_version = 0;
   auto ver_itr = bobserver_versions.begin();

   // The map should be sorted highest version to smallest, so we iterate until we hit the majority of bobservers on at least this version
   while( ver_itr != bobserver_versions.end() )
   {
      bobservers_on_version += ver_itr->second;

      if( bobservers_on_version >= bo_schedule_object.hardfork_required_bobservers )
      {
         majority_version = ver_itr->first;
         break;
      }

      ++ver_itr;
   }

   auto hf_itr = hardfork_version_votes.begin();

   while( hf_itr != hardfork_version_votes.end() )
   {
      if( hf_itr->second >= bo_schedule_object.hardfork_required_bobservers )
      {
         const auto& hfp = db.get_hardfork_property_object();
         if( hfp.next_hardfork != std::get<0>( hf_itr->first ) ||
             hfp.next_hardfork_time != std::get<1>( hf_itr->first ) ) {

            db.modify( hfp, [&]( hardfork_property_object& hpo )
            {
               hpo.next_hardfork = std::get<0>( hf_itr->first );
               hpo.next_hardfork_time = std::get<1>( hf_itr->first );
            } );
         }
         break;
      }

      ++hf_itr;
   }

   // We no longer have a majority
   if( hf_itr == hardfork_version_votes.end() )
   {
      db.modify( db.get_hardfork_property_object(), [&]( hardfork_property_object& hpo )
      {
         hpo.next_hardfork = hpo.current_hardfork_version;
      });
   }

   assert( num_bp + num_miners + num_timeshare == active_bobservers.size() );

   db.modify( bo_schedule_object, [&]( bobserver_schedule_object& _bso )
   {
      for( size_t i = 0; i < active_bobservers.size(); i++ )
      {
         _bso.current_shuffled_bobservers[i] = active_bobservers[i];
      }

      for( size_t i = active_bobservers.size(); i < SIGMAENGINE_NUM_BOBSERVERS; i++ )
      {
         _bso.current_shuffled_bobservers[i] = account_name_type();
      }

      _bso.num_scheduled_bobservers = std::max< uint8_t >( active_bobservers.size(), 1 );

      /// shuffle current shuffled bobservers
      auto now_hi = uint64_t(db.head_block_time().sec_since_epoch()) << 32;
      for( uint32_t i = 0; i < _bso.num_scheduled_bobservers; ++i )
      {
         /// High performance random generator
         /// http://xorshift.di.unimi.it/
         uint64_t k = now_hi + uint64_t(i)*2685821657736338717ULL;
         k ^= (k >> 12);
         k ^= (k << 25);
         k ^= (k >> 27);
         k *= 2685821657736338717ULL;

         uint32_t jmax = _bso.num_scheduled_bobservers - i;
         uint32_t j = i + k%jmax;
         std::swap( _bso.current_shuffled_bobservers[i],
                    _bso.current_shuffled_bobservers[j] );
      }

      _bso.next_shuffle_block_num = db.head_block_num() + _bso.num_scheduled_bobservers;
      _bso.majority_version = majority_version;
   } );

   dlog( "BP : active = ${active}", ( "active", active_bobservers ) );
   dlog( "BP : suffled_bp = ${suffled_bp}", ( "suffled_bp", bo_schedule_object.current_shuffled_bobservers ) );
}


/**
 *
 *  See @ref bobserver_object::virtual_last_update
 */
void update_bobserver_schedule(database& db)
{
   if( (db.head_block_num() % (SIGMAENGINE_NUM_BOBSERVERS)) == 0 )
   {
      update_bobserver_schedule4(db);
      return;
   }
}

} }
