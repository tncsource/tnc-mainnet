#pragma once
#include <sigmaengine/app/applied_operation.hpp>
#include <sigmaengine/app/state.hpp>

#include <sigmaengine/chain/database.hpp>
#include <sigmaengine/chain/sigmaengine_objects.hpp>
#include <sigmaengine/chain/sigmaengine_object_types.hpp>
#include <sigmaengine/chain/history_object.hpp>

#include <sigmaengine/bobserver/bobserver_plugin.hpp>

#include <fc/api.hpp>
#include <fc/optional.hpp>
#include <fc/variant_object.hpp>

#include <fc/network/ip.hpp>

#include <boost/container/flat_set.hpp>

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace sigmaengine { namespace app {

using namespace sigmaengine::chain;
using namespace sigmaengine::protocol;
using namespace std;

struct api_context;

struct scheduled_hardfork
{
   hardfork_version     hf_version;
   fc::time_point_sec   live_time;
};

class database_api_impl;

/**
 * @brief The database_api class implements the RPC API for the chain database.
 *
 * This API exposes accessors on the database which query state tracked by a blockchain validating node. This API is
 * read-only; all modifications to the database must be performed via transactions. Transactions are broadcast via
 * the @ref network_broadcast_api.
 */
class database_api
{
   public:
      database_api(const sigmaengine::app::api_context& ctx);
      ~database_api();

      ///////////////////
      // Subscriptions //
      ///////////////////

      void set_block_applied_callback( std::function<void(const variant& block_header)> cb );

      vector< account_name_type > get_active_bobservers()const;

      /////////////////////////////
      // Blocks and transactions //
      /////////////////////////////

      /**
       * @brief Retrieve a block header
       * @param block_num Height of the block whose header should be returned
       * @return header of the referenced block, or null if no matching block was found
       */
      optional<block_header> get_block_header(uint32_t block_num)const;

      /**
       * @brief Retrieve a full, signed block
       * @param block_num Height of the block to be returned
       * @return the referenced block, or null if no matching block was found
       */
      optional<signed_block_api_obj> get_block(uint32_t block_num)const;

      /**
       *  @brief Get sequence of operations included/generated within a particular block
       *  @param block_num Height of the block whose generated virtual operations should be returned
       *  @param only_virtual Whether to only include virtual operations in returned results (default: true)
       *  @return sequence of operations included/generated within the block
       */
      vector<applied_operation> get_ops_in_block(uint32_t block_num, bool only_virtual = true)const;

      /////////////
      // Globals //
      /////////////

      /**
       * @brief Retrieve compile-time constants
       */
      fc::variant_object get_config()const;

      /**
       * @brief Return a JSON description of object representations
       */
      std::string get_schema()const;

      /**
       * @brief Retrieve the current @ref dynamic_global_property_object
       */
      dynamic_global_property_api_obj  get_dynamic_global_properties()const;
      bobserver_schedule_api_obj       get_bobserver_schedule()const;
      hardfork_version                 get_hardfork_version()const;
      scheduled_hardfork               get_next_scheduled_hardfork()const;

      common_fund_api_obj              get_common_fund( string name )const;
      variant                          get_fund_info(string name) const;

      vector< savings_withdraw_api_obj > get_savings_withdraw_from( string account )const;
      vector< savings_withdraw_api_obj > get_savings_withdraw_to( string account )const;
      vector< fund_withdraw_api_obj > get_fund_withdraw_from( string fund_name, string account )const;
      vector< fund_withdraw_api_obj > get_fund_withdraw_list( string fund_name, uint32_t limit )const;

      dapp_reward_fund_api_object      get_dapp_reward_fund() const;

      //////////
      // Keys //
      //////////

      vector<set<string>> get_key_references( vector<public_key_type> key )const;

      //////////////
      // Accounts //
      //////////////

      vector< extended_account > get_accounts( vector< string > names ) const;

      /**
       *  @return all accounts that referr to the key or account id in their owner or active authorities.
       */
      vector<account_id_type> get_account_references( account_id_type account_id )const;

      /**
       * @brief Get a list of accounts by name
       * @param account_names Names of the accounts to retrieve
       * @return The accounts holding the provided names
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<account_api_obj>> lookup_account_names(const vector<string>& account_names)const;

      /**
       * @brief Get names and IDs for registered accounts
       * @param lower_bound_name Lower bound of the first name to return
       * @param limit Maximum number of results to return -- must not exceed 1000
       * @return Map of account names to corresponding IDs
       */
      set<string> lookup_accounts(const string& lower_bound_name, uint32_t limit)const;

      /**
       * @brief Get the total number of accounts registered with the blockchain
       */
      uint64_t get_account_count()const;

      vector< owner_authority_history_api_obj > get_owner_history( string account )const;
      optional< account_recovery_request_api_obj > get_recovery_request( string account ) const;

      ///////////////
      // Bobservers //
      ///////////////

      /**
       * @brief Get a list of bobservers by ID
       * @param bobserver_ids IDs of the bobservers to retrieve
       * @return The bobservers corresponding to the provided IDs
       *
       * This function has semantics identical to @ref get_objects
       */
      vector<optional<bobserver_api_obj>> get_bobservers(const vector<bobserver_id_type>& bobserver_ids)const;

      /**
       * @brief Get the bobserver owned by a given account
       * @param account The name of the account whose bobserver should be retrieved
       * @return The bobserver object, or null if the account does not have a bobserver
       */
      fc::optional< bobserver_api_obj > get_bobserver_by_account( string account_name )const;

      /**
       *  This method is used to fetch bobservers with pagination.
       *
       *  @return an array of `count` bobservers sorted by total votes after bobserver `from` with at most `limit' results.
       */
      vector< bobserver_api_obj > get_bobservers_by_vote( string from, uint32_t limit )const;

      /**
       * @brief Get names and IDs for registered bobservers
       * @param lower_bound_name Lower bound of the first name to return
       * @param limit Maximum number of results to return -- must not exceed 1000
       * @return Map of bobserver names to corresponding IDs
       */
      set<account_name_type> lookup_bobserver_accounts(const string& lower_bound_name, uint32_t limit)const;

      /**
       * Get names of registered BP(block producer)
       * @return BP list
       * */
      vector< bproducer_api_obj > lookup_bproducer_accounts()const;

      /**
       * @brief Get the total number of bobservers registered with the blockchain
       */
      uint64_t get_bobserver_count()const;
      
      bool has_hardfork( uint32_t hardfork )const;

      ////////////////////////////
      // Authority / validation //
      ////////////////////////////

      /// @brief Get a hexdump of the serialized binary form of a transaction
      std::string                   get_transaction_hex(const signed_transaction& trx)const;
      annotated_signed_transaction  get_transaction( transaction_id_type trx_id )const;

      /**
       *  This API will take a partially signed transaction and a set of public keys that the owner has the ability to sign for
       *  and return the minimal subset of public keys that should add signatures to the transaction.
       */
      set<public_key_type> get_required_signatures( const signed_transaction& trx, const flat_set<public_key_type>& available_keys )const;

      /**
       *  This method will return the set of all public keys that could possibly sign for a given transaction.  This call can
       *  be used by wallets to filter their set of public keys to just the relevant subset prior to calling @ref get_required_signatures
       *  to get the minimum subset.
       */
      set<public_key_type> get_potential_signatures( const signed_transaction& trx )const;

      /**
       * @return true of the @ref trx has all of the required signatures, otherwise throws an exception
       */
      bool           verify_authority( const signed_transaction& trx )const;

      /*
       * @return true if the signers have enough authority to authorize an account
       */
      bool           verify_account_authority( const string& name_or_id, const flat_set<public_key_type>& signers )const;

      /**
       *  Account operations have sequence numbers from 0 to N where N is the most recent operation. This method
       *  returns operations in the range [from-limit, from]
       *
       *  @param from - the absolute sequence number, -1 means most recent, limit is the number of operations before from.
       *  @param limit - the maximum number of items that can be queried (0 to 1000], must be less than from
       */
      map<uint32_t, applied_operation> get_account_history( string account, uint64_t from, uint32_t limit )const;

      vector< operation > get_history_by_opname( string account, string op_name )const; 

      ////////////////////////////
      // Handlers - not exposed //
      ////////////////////////////
      void on_api_startup();
      
      string get_auth_token( string account, string authtype)const;
      vector<account_auth_api_obj> get_auth_token_list( uint64_t from, uint32_t limit )const;

   private:
      std::shared_ptr< database_api_impl >   my;
};

} }

FC_REFLECT( sigmaengine::app::scheduled_hardfork, (hf_version)(live_time) );

FC_API(sigmaengine::app::database_api,
   // Subscriptions
   (set_block_applied_callback)

   // Blocks and transactions
   (get_block_header)
   (get_block)
   (get_ops_in_block)

   // Globals
   (get_config)
   (get_dynamic_global_properties)
   (get_bobserver_schedule)
   (get_hardfork_version)
   (get_next_scheduled_hardfork)

   (get_dapp_reward_fund)

   // Keys
   (get_key_references)

   // Accounts
   (get_accounts)
   (get_account_references)
   (lookup_account_names)
   (lookup_accounts)
   (get_account_count)
   (get_account_history)
   (get_history_by_opname) 
   (get_owner_history)
   (get_recovery_request)

   // Authority / validation
   (get_transaction_hex)
   (get_transaction)
   (get_required_signatures)
   (get_potential_signatures)
   (verify_authority)
   (verify_account_authority)

   // Bobservers
   (get_bobservers)
   (get_bobserver_by_account)
   (get_bobservers_by_vote)
   (lookup_bobserver_accounts)
   (get_bobserver_count)
   (get_active_bobservers)
   (lookup_bproducer_accounts)
   (has_hardfork)

   (get_auth_token)
   (get_auth_token_list)

   (get_common_fund)
   (get_fund_info)
   (get_savings_withdraw_from)  
   (get_savings_withdraw_to)
   (get_fund_withdraw_from)
   (get_fund_withdraw_list)
   
)
