#include <sigmaengine/app/api_context.hpp>
#include <sigmaengine/app/application.hpp>
#include <sigmaengine/app/database_api.hpp>

#include <sigmaengine/protocol/get_config.hpp>

#include <sigmaengine/chain/util/reward.hpp>

#include <fc/bloom_filter.hpp>
#include <fc/smart_ref_impl.hpp>
#include <fc/crypto/hex.hpp>

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>


#include <cctype>

#include <cfenv>
#include <iostream>

#define GET_REQUIRED_FEES_MAX_RECURSION 4

namespace sigmaengine { namespace app {

class database_api_impl;

class database_api_impl : public std::enable_shared_from_this<database_api_impl>
{
   public:
      database_api_impl( const sigmaengine::app::api_context& ctx  );
      ~database_api_impl();

      // Subscriptions
      void set_block_applied_callback( std::function<void(const variant& block_id)> cb );

      // Blocks and transactions
      optional<block_header> get_block_header(uint32_t block_num)const;
      optional<signed_block_api_obj> get_block(uint32_t block_num)const;
      vector<applied_operation> get_ops_in_block(uint32_t block_num, bool only_virtual)const;

      // Globals
      fc::variant_object get_config()const;
      dynamic_global_property_api_obj get_dynamic_global_properties()const;

      dapp_reward_fund_api_object get_dapp_reward_fund() const;

      // Keys
      vector<set<string>> get_key_references( vector<public_key_type> key )const;

      // Accounts
      vector< extended_account > get_accounts( vector< string > names )const;
      vector<account_id_type> get_account_references( account_id_type account_id )const;
      vector<optional<account_api_obj>> lookup_account_names(const vector<string>& account_names)const;
      set<string> lookup_accounts(const string& lower_bound_name, uint32_t limit)const;
      uint64_t get_account_count()const;

      // Bobservers
      vector<optional<bobserver_api_obj>> get_bobservers(const vector<bobserver_id_type>& bobserver_ids)const;
      fc::optional<bobserver_api_obj> get_bobserver_by_account( string account_name )const;
      set<account_name_type> lookup_bobserver_accounts(const string& lower_bound_name, uint32_t limit)const;
      vector< bproducer_api_obj > lookup_bproducer_accounts()const;
      uint64_t get_bobserver_count()const;

      bool has_hardfork( uint32_t hardfork )const;

      // Authority / validation
      std::string get_transaction_hex(const signed_transaction& trx)const;
      set<public_key_type> get_required_signatures( const signed_transaction& trx, const flat_set<public_key_type>& available_keys )const;
      set<public_key_type> get_potential_signatures( const signed_transaction& trx )const;
      bool verify_authority( const signed_transaction& trx )const;
      bool verify_account_authority( const string& name_or_id, const flat_set<public_key_type>& signers )const;

      // signal handlers
      void on_applied_block( const chain::signed_block& b );

      std::function<void(const fc::variant&)> _block_applied_callback;

      sigmaengine::chain::database&                _db;

      boost::signals2::scoped_connection       _block_applied_connection;

      bool _disable_get_block = false;
};

applied_operation::applied_operation() {}

applied_operation::applied_operation( const operation_object& op_obj )
 : trx_id( op_obj.trx_id ),
   block( op_obj.block ),
   trx_in_block( op_obj.trx_in_block ),
   op_in_trx( op_obj.op_in_trx ),
   virtual_op( op_obj.virtual_op ),
   timestamp( op_obj.timestamp )
{
   //fc::raw::unpack( op_obj.serialized_op, op );     // g++ refuses to compile this as ambiguous
   op = fc::raw::unpack< operation >( op_obj.serialized_op );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Subscriptions                                                    //
//                                                                  //
//////////////////////////////////////////////////////////////////////

void database_api::set_block_applied_callback( std::function<void(const variant& block_id)> cb )
{
   my->_db.with_read_lock( [&]()
   {
      my->set_block_applied_callback( cb );
   });
}

void database_api_impl::on_applied_block( const chain::signed_block& b )
{
   try
   {
      _block_applied_callback( fc::variant(signed_block_header(b) ) );
   }
   catch( ... )
   {
      _block_applied_connection.release();
   }
}

void database_api_impl::set_block_applied_callback( std::function<void(const variant& block_header)> cb )
{
   _block_applied_callback = cb;
   _block_applied_connection = connect_signal( _db.applied_block, *this, &database_api_impl::on_applied_block );
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Constructors                                                     //
//                                                                  //
//////////////////////////////////////////////////////////////////////

database_api::database_api( const sigmaengine::app::api_context& ctx )
   : my( new database_api_impl( ctx ) ) {}

database_api::~database_api() {}

database_api_impl::database_api_impl( const sigmaengine::app::api_context& ctx )
   : _db( *ctx.app.chain_database() )
{
   wlog("creating database api ${x}", ("x",int64_t(this)) );

   _disable_get_block = ctx.app._disable_get_block;
}

database_api_impl::~database_api_impl()
{
   elog("freeing database api ${x}", ("x",int64_t(this)) );
}

void database_api::on_api_startup() {}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Blocks and transactions                                          //
//                                                                  //
//////////////////////////////////////////////////////////////////////

optional<block_header> database_api::get_block_header(uint32_t block_num)const
{
   FC_ASSERT( !my->_disable_get_block, "get_block_header is disabled on this node." );

   return my->_db.with_read_lock( [&]()
   {
      return my->get_block_header( block_num );
   });
}

optional<block_header> database_api_impl::get_block_header(uint32_t block_num) const
{
   auto result = _db.fetch_block_by_number(block_num);
   if(result)
      return *result;
   return {};
}

optional<signed_block_api_obj> database_api::get_block(uint32_t block_num)const
{
   FC_ASSERT( !my->_disable_get_block, "get_block is disabled on this node." );

   return my->_db.with_read_lock( [&]()
   {
      return my->get_block( block_num );
   });
}

optional<signed_block_api_obj> database_api_impl::get_block(uint32_t block_num)const
{
   return _db.fetch_block_by_number(block_num);
}

vector<applied_operation> database_api::get_ops_in_block(uint32_t block_num, bool only_virtual)const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_ops_in_block( block_num, only_virtual );
   });
}

vector<applied_operation> database_api_impl::get_ops_in_block(uint32_t block_num, bool only_virtual)const
{
   const auto& idx = _db.get_index< operation_index >().indices().get< by_location >();   
   auto itr = idx.lower_bound( block_num );
   vector<applied_operation> result;
   applied_operation temp;
   while( itr != idx.end() && itr->block == block_num )
   {
      temp = *itr;
      if( !only_virtual || is_virtual_operation(temp.op) )
         result.push_back(temp);
      ++itr;
   }
   
   
   return result;
}


//////////////////////////////////////////////////////////////////////
//                                                                  //
// Globals                                                          //
//                                                                  //
//////////////////////////////////////////////////////////////////////=

fc::variant_object database_api::get_config()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_config();
   });
}

fc::variant_object database_api_impl::get_config()const
{
   return sigmaengine::protocol::get_config();
}

dynamic_global_property_api_obj database_api::get_dynamic_global_properties()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_dynamic_global_properties();
   });
}

dynamic_global_property_api_obj database_api_impl::get_dynamic_global_properties()const
{
   return dynamic_global_property_api_obj( _db.get( dynamic_global_property_id_type() ), _db );
}

bobserver_schedule_api_obj database_api::get_bobserver_schedule()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->_db.get(bobserver_schedule_id_type());
   });
}

hardfork_version database_api::get_hardfork_version()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->_db.get(hardfork_property_id_type()).current_hardfork_version;
   });
}

scheduled_hardfork database_api::get_next_scheduled_hardfork() const
{
   return my->_db.with_read_lock( [&]()
   {
      scheduled_hardfork shf;
      const auto& hpo = my->_db.get(hardfork_property_id_type());
      shf.hf_version = hpo.next_hardfork;
      shf.live_time = hpo.next_hardfork_time;
      return shf;
   });
}

common_fund_api_obj database_api::get_common_fund( string name )const
{
   return my->_db.with_read_lock( [&]()
   {
      auto fund = my->_db.find< common_fund_object, by_name >( name );
      FC_ASSERT( fund != nullptr, "Invalid reward fund name" );

      return *fund;
   });
}

variant database_api::get_fund_info(string name) const
{
   fc::mutable_variant_object result;
   common_fund_api_obj fund_obj = get_common_fund( name );
   result["fund_id"] = fund_obj.id;
   result["fund_name"] = fund_obj.name;
   result["fund_balance"] = fund_obj.fund_balance;
   result["fund_withdraw_ready"] = fund_obj.fund_withdraw_ready;
   result["01_month "] = fc::to_string(fund_obj.percent_interest[0][0]) + " | " + fc::to_string(fund_obj.percent_interest[1][0]);
   result["02_months"] = fc::to_string(fund_obj.percent_interest[0][1]) + " | " + fc::to_string(fund_obj.percent_interest[1][1]);
   result["03_months"] = fc::to_string(fund_obj.percent_interest[0][2]) + " | " + fc::to_string(fund_obj.percent_interest[1][2]);
   result["04_months"] = fc::to_string(fund_obj.percent_interest[0][3]) + " | " + fc::to_string(fund_obj.percent_interest[1][3]);
   result["05_months"] = fc::to_string(fund_obj.percent_interest[0][4]) + " | " + fc::to_string(fund_obj.percent_interest[1][4]);
   result["06_months"] = fc::to_string(fund_obj.percent_interest[0][5]) + " | " + fc::to_string(fund_obj.percent_interest[1][5]);
   result["07_months"] = fc::to_string(fund_obj.percent_interest[0][6]) + " | " + fc::to_string(fund_obj.percent_interest[1][6]);
   result["08_months"] = fc::to_string(fund_obj.percent_interest[0][7]) + " | " + fc::to_string(fund_obj.percent_interest[1][7]);
   result["09_months"] = fc::to_string(fund_obj.percent_interest[0][8]) + " | " + fc::to_string(fund_obj.percent_interest[1][8]);
   result["10_months"] = fc::to_string(fund_obj.percent_interest[0][9]) + " | " + fc::to_string(fund_obj.percent_interest[1][9]);
   result["11_months"] = fc::to_string(fund_obj.percent_interest[0][10]) + " | " + fc::to_string(fund_obj.percent_interest[1][10]);
   result["12_months"] = fc::to_string(fund_obj.percent_interest[0][11]) + " | " + fc::to_string(fund_obj.percent_interest[1][11]);
   result["last_update"] = fund_obj.last_update;

   return result;
}

vector< savings_withdraw_api_obj > database_api::get_savings_withdraw_from( string account )const
{
   return my->_db.with_read_lock( [&]()
   {
      vector<savings_withdraw_api_obj> result;

      const auto& from_rid_idx = my->_db.get_index< savings_withdraw_index >().indices().get< by_from_rid >();
      auto itr = from_rid_idx.lower_bound( account );
      while( itr != from_rid_idx.end() && itr->from == account ) {
         result.push_back( savings_withdraw_api_obj( *itr ) );
         ++itr;
      }
      return result;
   });
}

vector< savings_withdraw_api_obj > database_api::get_savings_withdraw_to( string account )const
{
   return my->_db.with_read_lock( [&]()
   {
      vector<savings_withdraw_api_obj> result;

      const auto& to_complete_idx = my->_db.get_index< savings_withdraw_index >().indices().get< by_to_complete >();
      auto itr = to_complete_idx.lower_bound( account );
      while( itr != to_complete_idx.end() && itr->to == account ) {
         result.push_back( savings_withdraw_api_obj( *itr ) );
         ++itr;
      }
      return result;
   });
}

vector< fund_withdraw_api_obj > database_api::get_fund_withdraw_from( string fund_name, string account )const
{
   return my->_db.with_read_lock( [&]()
   {
      vector<fund_withdraw_api_obj> result;

      const auto& from_idx = my->_db.get_index< fund_withdraw_index >().indices().get< by_from_complete >();
      auto itr = from_idx.lower_bound( boost::make_tuple( account, fund_name ) );
      while( itr != from_idx.end() && itr->from == account && to_string(itr->fund_name) == fund_name ) {
         result.push_back( fund_withdraw_api_obj( *itr ) );
         ++itr;
      }
      return result;
   });
}

vector< fund_withdraw_api_obj > database_api::get_fund_withdraw_list( string fund_name, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      FC_ASSERT( limit <= 10000, "Limit of ${l} is greater than maxmimum allowed", ("l",limit) );

      const auto& idx = my->_db.get_index< fund_withdraw_index >().indices().get< by_complete_from >();
      auto itr = idx.begin();

      vector<fund_withdraw_api_obj> result;

      while( itr != idx.end() && to_string(itr->fund_name) == fund_name && result.size() < limit ) {
         result.push_back( fund_withdraw_api_obj( *itr ) );
         ++itr;
      }
      return result;
   });
}



dapp_reward_fund_api_object database_api::get_dapp_reward_fund() const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_dapp_reward_fund();
   });
}

dapp_reward_fund_api_object database_api_impl::get_dapp_reward_fund() const
{
   return _db.get(dapp_reward_fund_id_type());
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Keys                                                             //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<set<string>> database_api::get_key_references( vector<public_key_type> key )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_key_references( key );
   });
}

/**
 *  @return all accounts that referr to the key or account id in their owner or active authorities.
 */
vector<set<string>> database_api_impl::get_key_references( vector<public_key_type> keys )const
{
   FC_ASSERT( false, "database_api::get_key_references has been deprecated. Please use account_by_key_api::get_key_references instead." );
   vector< set<string> > final_result;
   return final_result;
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Accounts                                                         //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector< extended_account > database_api::get_accounts( vector< string > names )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_accounts( names );
   });
}

vector< extended_account > database_api_impl::get_accounts( vector< string > names )const
{
   const auto& idx  = _db.get_index< account_index >().indices().get< by_name >();
   const auto& vidx = _db.get_index< bobserver_vote_index >().indices().get< by_account_bobserver >();
   vector< extended_account > results;

   for( auto name: names )
   {
      auto itr = idx.find( name );
      if ( itr != idx.end() )
      {
         results.push_back( extended_account( *itr, _db ) );

         auto vitr = vidx.lower_bound( boost::make_tuple( itr->id, bobserver_id_type() ) );
         while( vitr != vidx.end() && vitr->account == itr->id ) {
            results.back().bobserver_votes.insert(_db.get(vitr->bobserver).account);
            ++vitr;
         }
      }
   }

   return results;
}

vector<account_id_type> database_api::get_account_references( account_id_type account_id )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_references( account_id );
   });
}

vector<account_id_type> database_api_impl::get_account_references( account_id_type account_id )const
{
   /*const auto& idx = _db.get_index<account_index>();
   const auto& aidx = dynamic_cast<const primary_index<account_index>&>(idx);
   const auto& refs = aidx.get_secondary_index<sigmaengine::chain::account_member_index>();
   auto itr = refs.account_to_account_memberships.find(account_id);
   vector<account_id_type> result;

   if( itr != refs.account_to_account_memberships.end() )
   {
      result.reserve( itr->second.size() );
      for( auto item : itr->second ) result.push_back(item);
   }
   return result;*/
   FC_ASSERT( false, "database_api::get_account_references --- Needs to be refactored for sigmaengine." );
}

vector<optional<account_api_obj>> database_api::lookup_account_names(const vector<string>& account_names)const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->lookup_account_names( account_names );
   });
}

vector<optional<account_api_obj>> database_api_impl::lookup_account_names(const vector<string>& account_names)const
{
   vector<optional<account_api_obj> > result;
   result.reserve(account_names.size());

   for( auto& name : account_names )
   {
      auto itr = _db.find< account_object, by_name >( name );

      if( itr )
      {
         result.push_back( account_api_obj( *itr, _db ) );
      }
      else
      {
         result.push_back( optional< account_api_obj>() );
      }
   }

   return result;
}

set<string> database_api::lookup_accounts(const string& lower_bound_name, uint32_t limit)const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->lookup_accounts( lower_bound_name, limit );
   });
}

set<string> database_api_impl::lookup_accounts(const string& lower_bound_name, uint32_t limit)const
{
   FC_ASSERT( limit <= 1000 );
   const auto& accounts_by_name = _db.get_index<account_index>().indices().get<by_name>();
   set<string> result;

   for( auto itr = accounts_by_name.lower_bound(lower_bound_name);
        limit-- && itr != accounts_by_name.end();
        ++itr )
   {
      result.insert(itr->name);
   }

   return result;
}

uint64_t database_api::get_account_count()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_account_count();
   });
}

uint64_t database_api_impl::get_account_count()const
{
   return _db.get_index<account_index>().indices().size();
}

vector< owner_authority_history_api_obj > database_api::get_owner_history( string account )const
{
   return my->_db.with_read_lock( [&]()
   {
      vector< owner_authority_history_api_obj > results;

      const auto& hist_idx = my->_db.get_index< owner_authority_history_index >().indices().get< by_account >();
      auto itr = hist_idx.lower_bound( account );

      while( itr != hist_idx.end() && itr->account == account )
      {
         results.push_back( owner_authority_history_api_obj( *itr ) );
         ++itr;
      }

      return results;
   });
}

optional< account_recovery_request_api_obj > database_api::get_recovery_request( string account )const
{
   return my->_db.with_read_lock( [&]()
   {
      optional< account_recovery_request_api_obj > result;

      const auto& rec_idx = my->_db.get_index< account_recovery_request_index >().indices().get< by_account >();
      auto req = rec_idx.find( account );

      if( req != rec_idx.end() )
         result = account_recovery_request_api_obj( *req );

      return result;
   });
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Bobservers                                                        //
//                                                                  //
//////////////////////////////////////////////////////////////////////

vector<optional<bobserver_api_obj>> database_api::get_bobservers(const vector<bobserver_id_type>& bobserver_ids)const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_bobservers( bobserver_ids );
   });
}

vector<optional<bobserver_api_obj>> database_api_impl::get_bobservers(const vector<bobserver_id_type>& bobserver_ids)const
{
   vector<optional<bobserver_api_obj>> result; result.reserve(bobserver_ids.size());
   std::transform(bobserver_ids.begin(), bobserver_ids.end(), std::back_inserter(result),
                  [this](bobserver_id_type id) -> optional<bobserver_api_obj> {
      if(auto o = _db.find(id))
         return *o;
      return {};
   });
   return result;
}

fc::optional<bobserver_api_obj> database_api::get_bobserver_by_account( string account_name ) const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_bobserver_by_account( account_name );
   });
}

vector< bobserver_api_obj > database_api::get_bobservers_by_vote( string from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      //idump((from)(limit));
      FC_ASSERT( limit <= 100 );

      vector<bobserver_api_obj> result;
      result.reserve(limit);

      const auto& name_idx = my->_db.get_index< bobserver_index >().indices().get< by_name >();
      const auto& vote_idx = my->_db.get_index< bobserver_index >().indices().get< by_vote_name >();

      auto itr = vote_idx.begin();
      if( from.size() ) {
         auto nameitr = name_idx.find( from );
         FC_ASSERT( nameitr != name_idx.end(), "invalid bobserver name ${n}", ("n",from) );
         itr = vote_idx.iterator_to( *nameitr );
      }

      while( itr != vote_idx.end()  &&
            result.size() < limit &&
            itr->votes > 0 )
      {
         result.push_back( bobserver_api_obj( *itr ) );
         ++itr;
      }
      return result;
   });
}

fc::optional<bobserver_api_obj> database_api_impl::get_bobserver_by_account( string account_name ) const
{
   const auto& idx = _db.get_index< bobserver_index >().indices().get< by_name >();
   auto itr = idx.find( account_name );
   if( itr != idx.end() )
      return bobserver_api_obj( *itr );
   return {};
}

set< account_name_type > database_api::lookup_bobserver_accounts( const string& lower_bound_name, uint32_t limit ) const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->lookup_bobserver_accounts( lower_bound_name, limit );
   });
}

set< account_name_type > database_api_impl::lookup_bobserver_accounts( const string& lower_bound_name, uint32_t limit ) const
{
   FC_ASSERT( limit <= 1000 );
   const auto& bobservers_by_id = _db.get_index< bobserver_index >().indices().get< by_id >();

   // get all the names and look them all up, sort them, then figure out what
   // records to return.  This could be optimized, but we expect the
   // number of bobservers to be few and the frequency of calls to be rare
   set< account_name_type > bobservers_by_account_name;
   for ( const bobserver_api_obj& bobserver : bobservers_by_id )
      if ( !bobserver.is_excepted 
         && bobserver.account >= lower_bound_name ) // we can ignore anything below lower_bound_name
         bobservers_by_account_name.insert( bobserver.account );

   auto end_iter = bobservers_by_account_name.begin();
   while ( end_iter != bobservers_by_account_name.end() && limit-- )
       ++end_iter;
   bobservers_by_account_name.erase( end_iter, bobservers_by_account_name.end() );
   return bobservers_by_account_name;
}

vector< bproducer_api_obj > database_api::lookup_bproducer_accounts()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->lookup_bproducer_accounts();
   });
}

vector< bproducer_api_obj > database_api_impl::lookup_bproducer_accounts()const
{
   const auto& idx = _db.get_index< bobserver_index >().indices().get< by_name >();

   auto itr = idx.begin();
   vector< bproducer_api_obj > results;
   while( itr != idx.end() ) {
      if( itr->is_bproducer ) {
         results.emplace_back( *itr );
      }
      itr++;
   }

   return results;
}

uint64_t database_api::get_bobserver_count()const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_bobserver_count();
   });
}

uint64_t database_api_impl::get_bobserver_count()const
{
   return _db.get_index<bobserver_index>().indices().size();
}

bool database_api::has_hardfork( uint32_t hardfork  ) const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->has_hardfork( hardfork );
   });
}

bool database_api_impl::has_hardfork( uint32_t hardfork )const
{
   return _db.has_hardfork(hardfork);
}

//////////////////////////////////////////////////////////////////////
//                                                                  //
// Authority / validation                                           //
//                                                                  //
//////////////////////////////////////////////////////////////////////

std::string database_api::get_transaction_hex(const signed_transaction& trx)const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_transaction_hex( trx );
   });
}

std::string database_api_impl::get_transaction_hex(const signed_transaction& trx)const
{
   return fc::to_hex(fc::raw::pack(trx));
}

set<public_key_type> database_api::get_required_signatures( const signed_transaction& trx, const flat_set<public_key_type>& available_keys )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_required_signatures( trx, available_keys );
   });
}

set<public_key_type> database_api_impl::get_required_signatures( const signed_transaction& trx, const flat_set<public_key_type>& available_keys )const
{
//   wdump((trx)(available_keys));
   auto result = trx.get_required_signatures( SIGMAENGINE_CHAIN_ID,
                                              available_keys,
                                              [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).active  ); },
                                              [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).owner   ); },
                                              [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).posting ); },
                                              SIGMAENGINE_MAX_SIG_CHECK_DEPTH );
//   wdump((result));
   return result;
}

set<public_key_type> database_api::get_potential_signatures( const signed_transaction& trx )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->get_potential_signatures( trx );
   });
}

set<public_key_type> database_api_impl::get_potential_signatures( const signed_transaction& trx )const
{
//   wdump((trx));
   set<public_key_type> result;
   trx.get_required_signatures(
      SIGMAENGINE_CHAIN_ID,
      flat_set<public_key_type>(),
      [&]( account_name_type account_name )
      {
         const auto& auth = _db.get< account_authority_object, by_account >(account_name).active;
         for( const auto& k : auth.get_keys() )
            result.insert(k);
         return authority( auth );
      },
      [&]( account_name_type account_name )
      {
         const auto& auth = _db.get< account_authority_object, by_account >(account_name).owner;
         for( const auto& k : auth.get_keys() )
            result.insert(k);
         return authority( auth );
      },
      [&]( account_name_type account_name )
      {
         const auto& auth = _db.get< account_authority_object, by_account >(account_name).posting;
         for( const auto& k : auth.get_keys() )
            result.insert(k);
         return authority( auth );
      },
      SIGMAENGINE_MAX_SIG_CHECK_DEPTH
   );

//   wdump((result));
   return result;
}

bool database_api::verify_authority( const signed_transaction& trx ) const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->verify_authority( trx );
   });
}

bool database_api_impl::verify_authority( const signed_transaction& trx )const
{
   trx.verify_authority( SIGMAENGINE_CHAIN_ID,
                         [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).active  ); },
                         [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).owner   ); },
                         [&]( string account_name ){ return authority( _db.get< account_authority_object, by_account >( account_name ).posting ); },
                         SIGMAENGINE_MAX_SIG_CHECK_DEPTH );
   return true;
}

bool database_api::verify_account_authority( const string& name_or_id, const flat_set<public_key_type>& signers )const
{
   return my->_db.with_read_lock( [&]()
   {
      return my->verify_account_authority( name_or_id, signers );
   });
}

bool database_api_impl::verify_account_authority( const string& name, const flat_set<public_key_type>& keys )const
{
   FC_ASSERT( name.size() > 0);
   auto account = _db.find< account_object, by_name >( name );
   FC_ASSERT( account, "no such account" );

   /// reuse trx.verify_authority by creating a dummy transfer
   signed_transaction trx;
   transfer_operation op;
   op.from = account->name;
   trx.operations.emplace_back(op);

   return verify_authority( trx );
}

u256 to256( const fc::uint128& t )
{
   u256 result( t.high_bits() );
   result <<= 65;
   result += t.low_bits();
   return result;
}

asset database_api::get_total_supply() const
{
   return my->_db.with_read_lock( [&]()
   {
      const auto& null_account = my->_db.get_account( SIGMAENGINE_NULL_ACCOUNT );
      asset miss_balance( 0, SGT_SYMBOL );

      if( null_account.balance.amount > 0 )
      {
         miss_balance += null_account.balance;
      }

      if( null_account.savings_balance.amount > 0 )
      {
         miss_balance += null_account.savings_balance;
      }

      asset current_supply = my->_db.get_dynamic_global_properties().current_supply;
      asset total_supply = current_supply + miss_balance;

      return total_supply;
      
   });
}

asset database_api::get_dapp_transaction_fee() const
{
   return my->_db.with_read_lock( [&]()
   {
      asset fee = my->_db.get_dynamic_global_properties().dapp_transaction_fee;
      
      return fee;
      
   });
}

map< uint32_t, account_balance_api_obj > database_api::get_balance_rank( uint64_t from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      FC_ASSERT( limit <= 10000, "Limit of ${l} is greater than maxmimum allowed", ("l",limit) );

      const auto& idx = my->_db.get_index< account_index >().indices().get< by_balance >();

      auto itr = idx.begin();
      auto end = idx.end();

      uint32_t index = 0;
      if ( from > 0 )
      {
         while( itr != end && from-- )
         {
            ++itr;
            index++;
         }
      }

      map<uint32_t, account_balance_api_obj> result;
      account_balance_api_obj temp;

      while( itr != end && limit-- )
      {
         temp = account_balance_api_obj(itr->name, itr->balance);
         result[index] = temp;
         ++itr;
         index++;
      }
      
      return result;
   });
}

map< uint32_t, optional<signed_block_api_obj>> database_api::get_block_range(uint32_t block_num, uint16_t num)const
{
   FC_ASSERT( !my->_disable_get_block, "get_block is disabled on this node." );
   FC_ASSERT( num < 1000, "num lass than 1000." );

   return my->_db.with_read_lock( [&]()
   {
      map<uint32_t, optional<signed_block_api_obj>> result;
      for ( uint32_t i = 0 ; i < num ; i++ )
      {
         uint32_t b = block_num - i;
         if ( b > my->_db.head_block_num() )
         {
            continue;
         }

         result[b] = my->get_block(b);
      }
   
      return result;
   });
}

map< uint32_t, applied_operation > database_api::get_operation_list( uint64_t from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      FC_ASSERT( limit <= 10000, "Limit of ${l} is greater than maxmimum allowed", ("l",limit) );
      //FC_ASSERT( from >= 0, "From must be greater than -1" );

      const auto& idx = my->_db.get_index< operation_index >().indices().get< by_location >();

      auto itr = idx.rbegin();
      auto end = idx.rend();
      
      uint32_t start = from;
      
      /*
      auto itr = idx.begin();
      auto end = idx.end();

      uint32_t max = idx.size();
      //ilog("get_opeartion_list size ${s}",("s",max));

      uint32_t start = max - limit;
      if ( from < my->_db.head_block_num() )
      {
         start = from;
      }
      */

      uint32_t index = 0;
      while( itr != end && start-- )
      {
         ++itr;
         index++;
      }

      map<uint32_t, applied_operation> result;
      applied_operation temp;
      
      while( itr != end && limit-- )
      {
         //ilog(" itr opeartion is ${b} ${op}",("b", itr->block)("op",itr->trx_id));

         temp = *itr;
         result[index] = temp;
         ++itr;
         index++;
      }
      
      return result;
   });
}

map< uint32_t, applied_operation > database_api::get_account_transfer_history( string account, uint64_t from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      FC_ASSERT( limit <= 10000, "Limit of ${l} is greater than maxmimum allowed", ("l",limit) );
      FC_ASSERT( from >= limit, "From must be greater than limit" );

      const auto& idx = my->_db.get_index<account_history_index>().indices().get<by_op_tag>();
      auto itr = idx.lower_bound( boost::make_tuple( account, operation::tag<transfer_operation>::value, from ) );
      auto end = idx.upper_bound( boost::make_tuple( account, operation::tag<transfer_operation>::value, std::max( int64_t(0), int64_t(itr->op_seq)-limit ) ) );

      map<uint32_t, applied_operation> result;
      while( itr != end )
      {
         result[itr->op_seq] = my->_db.get(itr->op);
         ++itr;
      }
      return result;

   
   /*
      FC_ASSERT( limit <= 10000, "Limit of ${l} is greater than maxmimum allowed", ("l",limit) );
      
      const auto& idx = my->_db.get_index<account_history_index>().indices().get<by_account>();
      FC_ASSERT( from < idx.size(), "From(${f}) must be less than ${s}", ("f", from)("s", idx.size()) );
      
      auto itr = idx.lower_bound( boost::make_tuple( account, uint64_t(-1) ) );
      auto end = idx.upper_bound( boost::make_tuple( account, uint64_t(0) ) );

      uint32_t index = 0;
      uint64_t start = from;

      map<uint32_t, applied_operation> result;
      applied_operation op;
      while( itr != end && limit > 0 )
      {
         applied_operation op = my->_db.get(itr->op);

         if( op.op.which() == operation::tag<transfer_operation>::value ) 
         {
            auto& temp = op.op.get<transfer_operation>();
            account_name_type name = account;
            
            if ( type == 1 )
            {
               if ( temp.from == name )
               {
                  if ( start > 0 )
                  {
                     start --;
                  }
                  else
                  {
                     result[index] = op;
                     limit--;
                  }
                  index++;
               }
            }
            else if ( type == 2 )
            {
               if ( temp.to == name )
               {
                  if ( start > 0 )
                  {
                     start --;
                  }
                  else
                  {
                     result[index] = op;
                     limit--;
                  }
                  index++;
               }
            }
            else if ( type == 0 )
            {
               if ( start > 0 )
               {
                  start --;
               }
               else
               {
                  result[index] = op;
                  limit--;
               }
               index++;
            }
         }
         
         ++itr;
      }
      return result;
      */
   });
}

map< uint32_t, applied_operation > database_api::get_account_history( string account, uint64_t from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      FC_ASSERT( limit <= 10000, "Limit of ${l} is greater than maxmimum allowed", ("l",limit) );
      FC_ASSERT( from >= limit, "From must be greater than limit" );
   //   idump((account)(from)(limit));
      const auto& idx = my->_db.get_index<account_history_index>().indices().get<by_account>();
      auto itr = idx.lower_bound( boost::make_tuple( account, from ) );
   //   if( itr != idx.end() ) idump((*itr));
      auto end = idx.upper_bound( boost::make_tuple( account, std::max( int64_t(0), int64_t(itr->sequence)-limit ) ) );
   //   if( end != idx.end() ) idump((*end));

      map<uint32_t, applied_operation> result;
      while( itr != end )
      {
         result[itr->sequence] = my->_db.get(itr->op);
         ++itr;
      }
      return result;
   });
}

vector< operation > database_api::get_history_by_opname( string account, string op_name )const 
{
   return my->_db.with_read_lock( [&]()
   {
      const auto& idx = my->_db.get_index<account_history_index>().indices().get<by_account>();
      auto itr = idx.lower_bound(boost::make_tuple( account, -1 ));
      auto end = idx.upper_bound( boost::make_tuple( account, std::max( int64_t(0), int64_t(itr->sequence)-10000 ) ) );
      vector<operation> result;
      applied_operation temp;

      while( itr != end )
      {
         temp = my->_db.get(itr->op);
         const auto& tempop = temp.op;
         fc::string opname = get_op_name(tempop);
         if (opname.find(op_name.c_str(),0) != fc::string::npos) {
            result.push_back(tempop);
         }
         ++itr;
      }
      return result;
   });
}

uint32_t database_api::get_free_memory()
{
   uint32_t free_gb = uint32_t( my->_db.get_free_memory() / (1024*1024*1024) );

   return free_gb;
}

vector< account_name_type > database_api::get_active_bobservers()const
{
   return my->_db.with_read_lock( [&]()
   {
      const auto& wso = my->_db.get_bobserver_schedule_object();
      size_t n = wso.current_shuffled_bobservers.size();
      vector< account_name_type > result;
      result.reserve( n );
      for( size_t i=0; i<n; i++ )
         result.push_back( wso.current_shuffled_bobservers[i] );
      return result;
   });
}

uint64_t database_api::get_transaction_count(uint32_t block)const
{
   return my->_db.with_read_lock( [&]()
   {
      const auto& idx = my->_db.get_index< operation_index >().indices().get< by_location >();

      if ( block > 0 )
      {
         FC_ASSERT( (my->_db.head_block_num() - block) <= 30000, "block ${l} is lass than head_block_num - 30000", ("l",block) );
         auto itr = idx.rbegin();
         auto end = idx.rend();

         uint64_t cnt = 0;
         while( itr != end && itr->block > block)
         {
            cnt++;
            ++itr;
         }

         return cnt;
      }
      else
      {
         return idx.size();
      }
   });
}

annotated_signed_transaction database_api::get_transaction( transaction_id_type id )const
{
#ifdef SKIP_BY_TX_ID
   FC_ASSERT( false, "This node's operator has disabled operation indexing by transaction_id" );
#else
   return my->_db.with_read_lock( [&](){
      const auto& idx = my->_db.get_index<operation_index>().indices().get<by_transaction_id>();
      auto itr = idx.lower_bound( id );
      if( itr != idx.end() && itr->trx_id == id ) {
         auto blk = my->_db.fetch_block_by_number( itr->block );
         FC_ASSERT( blk.valid() );
         FC_ASSERT( blk->transactions.size() > itr->trx_in_block );
         annotated_signed_transaction result = blk->transactions[itr->trx_in_block];
         result.block_num       = itr->block;
         result.transaction_num = itr->trx_in_block;
         return result;
      }
      
      FC_ASSERT( false, "Unknown Transaction ${t}", ("t",id));
   });
#endif
}

string database_api::get_auth_token( string account, string authtype)const
{
   return my->_db.with_read_lock( [&]()
   {
      account_auth_api_obj result;

      const auto& auth_idx = my->_db.get_index< account_auth_index >().indices().get< by_auth_type >();
      auto itr = auth_idx.find( boost::make_tuple( account, authtype ) );
      result = account_auth_api_obj( *itr );
      return result.auth_token;
   });
}

vector<account_auth_api_obj> database_api::get_auth_token_list( uint64_t from, uint32_t limit )const
{
   return my->_db.with_read_lock( [&]()
   {
      FC_ASSERT( limit <= 10000, "Limit of ${l} is greater than maxmimum allowed", ("l",limit) );
      FC_ASSERT( from >= 0, "From must be greater than -1" );

      const auto& auth_idx = my->_db.get_index< account_auth_index >().indices().get< by_id >();
      auto itr = auth_idx.begin();
      auto end = auth_idx.end();

      while( itr != end && from--)
      {
         ++itr;
      }

      vector <account_auth_api_obj> result;
      while( itr != end && limit--)
      {
         result.push_back(account_auth_api_obj( *itr ));
         ++itr;
      }
      return result;
   });
}

} } // sigmaengine::app
