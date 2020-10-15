#pragma once
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>

//#include <graphene/db2/database.hpp>
#include <chainbase/chainbase.hpp>

#include <sigmaengine/protocol/types.hpp>
#include <sigmaengine/protocol/authority.hpp>


namespace sigmaengine { namespace chain {

namespace bip = chainbase::bip;
using namespace boost::multi_index;

using boost::multi_index_container;

using chainbase::object;
using chainbase::oid;
using chainbase::allocator;

using sigmaengine::protocol::block_id_type;
using sigmaengine::protocol::transaction_id_type;
using sigmaengine::protocol::chain_id_type;
using sigmaengine::protocol::account_name_type;
using sigmaengine::protocol::share_type;

typedef bip::basic_string< char, std::char_traits< char >, allocator< char > > shared_string;
inline std::string to_string( const shared_string& str ) { return std::string( str.begin(), str.end() ); }
inline void from_string( shared_string& out, const string& in ){ out.assign( in.begin(), in.end() ); }

typedef bip::vector< char, allocator< char > > buffer_type;

struct by_id;

enum object_type
{
   dynamic_global_property_object_type,
   account_object_type,
   account_authority_object_type,
   bobserver_object_type,
   transaction_object_type,
   block_summary_object_type,
   bobserver_schedule_object_type,
   bobserver_vote_object_type,
   operation_object_type,
   account_history_object_type,
   hardfork_property_object_type,
   owner_authority_history_object_type,
   account_recovery_request_object_type,
   change_recovery_account_request_object_type,
   decline_voting_rights_request_object_type,
   block_stats_object_type,
   account_auth_object_type,
   savings_withdraw_object_type,
   common_fund_object_type,
   fund_withdraw_object_type,
   dapp_reward_fund_object_type
};

class dynamic_global_property_object;
class account_object;
class account_authority_object;
class bobserver_object;
class transaction_object;
class block_summary_object;
class bobserver_schedule_object;
class bobserver_vote_object;
class operation_object;
class account_history_object;
class hardfork_property_object;
class owner_authority_history_object;
class account_recovery_request_object;
class change_recovery_account_request_object;
class savings_withdraw_object;
class decline_voting_rights_request_object;
class block_stats_object;
class common_fund_object;
class account_auth_object;
class fund_withdraw_object;
class dapp_reward_fund_object;

typedef oid< dynamic_global_property_object         > dynamic_global_property_id_type;
typedef oid< account_object                         > account_id_type;
typedef oid< account_authority_object               > account_authority_id_type;
typedef oid< bobserver_object                       > bobserver_id_type;
typedef oid< transaction_object                     > transaction_object_id_type;
typedef oid< block_summary_object                   > block_summary_id_type;
typedef oid< bobserver_schedule_object              > bobserver_schedule_id_type;
typedef oid< bobserver_vote_object                  > bobserver_vote_id_type;
typedef oid< operation_object                       > operation_id_type;
typedef oid< account_history_object                 > account_history_id_type;
typedef oid< hardfork_property_object               > hardfork_property_id_type;
typedef oid< owner_authority_history_object         > owner_authority_history_id_type;
typedef oid< account_recovery_request_object        > account_recovery_request_id_type;
typedef oid< change_recovery_account_request_object > change_recovery_account_request_id_type;
typedef oid< savings_withdraw_object                > savings_withdraw_id_type;
typedef oid< decline_voting_rights_request_object   > decline_voting_rights_request_id_type;
typedef oid< block_stats_object                     > block_stats_id_type;
typedef oid< common_fund_object                     > common_fund_id_type;
typedef oid< fund_withdraw_object                   > fund_withdraw_id_type;
typedef oid< account_auth_object                    > account_auth_id_type;
typedef oid< dapp_reward_fund_object                > dapp_reward_fund_id_type;

enum bandwidth_type
{
   post    ///< Rate limiting posting reward eligibility over time
};

} } //sigmaengine::chain

namespace fc
{
   class variant;
   inline void to_variant( const sigmaengine::chain::shared_string& s, variant& var )
   {
      var = fc::string( sigmaengine::chain::to_string( s ) );
   }

   inline void from_variant( const variant& var, sigmaengine::chain::shared_string& s )
   {
      auto str = var.as_string();
      s.assign( str.begin(), str.end() );
   }

   template<typename T>
   void to_variant( const chainbase::oid<T>& var,  variant& vo )
   {
      vo = var._id;
   }
   template<typename T>
   void from_variant( const variant& vo, chainbase::oid<T>& var )
   {
      var._id = vo.as_int64();
   }

   namespace raw {
      template<typename Stream, typename T>
      inline void pack( Stream& s, const chainbase::oid<T>& id )
      {
         s.write( (const char*)&id._id, sizeof(id._id) );
      }
      template<typename Stream, typename T>
      inline void unpack( Stream& s, chainbase::oid<T>& id )
      {
         s.read( (char*)&id._id, sizeof(id._id));
      }
   }

   namespace raw
   {
      namespace bip = chainbase::bip;
      using chainbase::allocator;

      template< typename T > inline void pack( sigmaengine::chain::buffer_type& raw, const T& v )
      {
         auto size = pack_size( v );
         raw.resize( size );
         datastream< char* > ds( raw.data(), size );
         pack( ds, v );
      }

      template< typename T > inline void unpack( const sigmaengine::chain::buffer_type& raw, T& v )
      {
         datastream< const char* > ds( raw.data(), raw.size() );
         unpack( ds, v );
      }

      template< typename T > inline T unpack( const sigmaengine::chain::buffer_type& raw )
      {
         T v;
         datastream< const char* > ds( raw.data(), raw.size() );
         unpack( ds, v );
         return v;
      }
   }
}

namespace fc {

}

FC_REFLECT_ENUM( sigmaengine::chain::object_type,
                 (dynamic_global_property_object_type)
                 (account_object_type)
                 (account_authority_object_type)
                 (bobserver_object_type)
                 (transaction_object_type)
                 (block_summary_object_type)
                 (bobserver_schedule_object_type)
                 (bobserver_vote_object_type)
                 (operation_object_type)
                 (account_history_object_type)
                 (hardfork_property_object_type)
                 (owner_authority_history_object_type)
                 (account_recovery_request_object_type)
                 (change_recovery_account_request_object_type)
                 (decline_voting_rights_request_object_type)
                 (block_stats_object_type)
                 (account_auth_object_type)
                 (savings_withdraw_object_type)
                 (common_fund_object_type)
                 (fund_withdraw_object_type)
                 (dapp_reward_fund_object_type)
               )

FC_REFLECT_TYPENAME( sigmaengine::chain::shared_string )
FC_REFLECT_TYPENAME( sigmaengine::chain::buffer_type )
