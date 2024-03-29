/*
 * Copyright (c) 2015 Cryptonomex, Inc., and contributors.
 */
#pragma once
#include <sigmaengine/chain/global_property_object.hpp>
#include <sigmaengine/chain/hardfork_property_object.hpp>
#include <sigmaengine/chain/node_property_object.hpp>
#include <sigmaengine/chain/fork_database.hpp>
#include <sigmaengine/chain/block_log.hpp>
#include <sigmaengine/chain/operation_notification.hpp>

#include <sigmaengine/protocol/protocol.hpp>
#include <sigmaengine/protocol/hardfork.hpp>

//#include <graphene/db2/database.hpp>
#include <fc/signals.hpp>

#include <fc/log/logger.hpp>

#include <map>

namespace sigmaengine { namespace chain {

   using sigmaengine::protocol::signed_transaction;
   using sigmaengine::protocol::operation;
   using sigmaengine::protocol::authority;
   using sigmaengine::protocol::asset;
   using sigmaengine::protocol::asset_symbol_type;
   using sigmaengine::protocol::price;

   class database_impl;
   class custom_operation_interpreter;

   namespace util {
      struct comment_reward_context;
   }

   /**
    *   @class database
    *   @brief tracks the blockchain state in an extensible manner
    */
   class database : public chainbase::database
   {
      public:
         database();
         ~database();

         bool is_producing()const { return _is_producing; }
         void set_producing( bool p ) { _is_producing = p;  }
         bool _is_producing = false;

         bool _log_hardforks = true;

         enum validation_steps
         {
            skip_nothing                = 0,
            skip_bobserver_signature    = 1 << 0,    ///< used while reindexing
            skip_transaction_signatures = 1 << 1,    ///< used by non-bobserver nodes
            skip_transaction_dupe_check = 1 << 2,    ///< used while reindexing
            skip_fork_db                = 1 << 3,    ///< used while reindexing
            skip_block_size_check       = 1 << 4,    ///< used when applying locally generated transactions
            skip_tapos_check            = 1 << 5,    ///< used while reindexing -- note this skips expiration check as well
            skip_authority_check        = 1 << 6,    ///< used while reindexing -- disables any checking of authority on transactions
            skip_merkle_check           = 1 << 7,    ///< used while reindexing
            skip_undo_history_check     = 1 << 8,    ///< used while reindexing
            skip_bobserver_schedule_check = 1 << 9,  ///< used while reindexing
            skip_validate               = 1 << 10,   ///< used prior to checkpoint, skips validate() call on transaction
            skip_validate_invariants    = 1 << 11,   ///< used to skip database invariant check on block application
            skip_undo_block             = 1 << 12,   ///< used to skip undo db on reindex
            skip_block_log              = 1 << 13    ///< used to skip block logging on reindex
         };

         /**
          * @brief Open a database, creating a new one if necessary
          *
          * Opens a database in the specified directory. If no initialized database is found the database
          * will be initialized with the default state.
          *
          * @param data_dir Path to open or create database in
          */
         void open( const fc::path& data_dir, const fc::path& shared_mem_dir, uint64_t initial_supply = SIGMAENGINE_INIT_SUPPLY, uint64_t shared_file_size = 0, uint32_t chainbase_flags = 0 );

         /**
          * @brief Rebuild object graph from block history and open detabase
          *
          * This method may be called after or instead of @ref database::open, and will rebuild the object graph by
          * replaying blockchain history. When this method exits successfully, the database will be open.
          */
         void reindex( const fc::path& data_dir, const fc::path& shared_mem_dir, uint64_t shared_file_size = (1024l*1024l*1024l*8l) );

         /**
          * @brief wipe Delete database from disk, and potentially the raw chain as well.
          * @param include_blocks If true, delete the raw chain as well as the database.
          *
          * Will close the database before wiping. Database will be closed when this function returns.
          */
         void wipe(const fc::path& data_dir, const fc::path& shared_mem_dir, bool include_blocks);
         void close(bool rewind = true);

         //////////////////// db_block.cpp ////////////////////

         /**
          *  @return true if the block is in our fork DB or saved to disk as
          *  part of the official chain, otherwise return false
          */
         bool                       is_known_block( const block_id_type& id )const;
         bool                       is_known_transaction( const transaction_id_type& id )const;
         block_id_type              find_block_id_for_num( uint32_t block_num )const;
         block_id_type              get_block_id_for_num( uint32_t block_num )const;
         optional<signed_block>     fetch_block_by_id( const block_id_type& id )const;
         optional<signed_block>     fetch_block_by_number( uint32_t num )const;
         const signed_transaction   get_recent_transaction( const transaction_id_type& trx_id )const;
         std::vector<block_id_type> get_block_ids_on_fork(block_id_type head_of_fork) const;

         chain_id_type             get_chain_id()const;

         const bobserver_object&  get_bobserver(  const account_name_type& name )const;
         const bobserver_object*  find_bobserver( const account_name_type& name )const;

         const account_object&  get_account(  const account_name_type& name )const;
         const account_object*  find_account( const account_name_type& name )const;

         const savings_withdraw_object& get_savings_withdraw(  const account_name_type& owner, uint32_t request_id )const;
         const savings_withdraw_object* find_savings_withdraw( const account_name_type& owner, uint32_t request_id )const;

         const fund_withdraw_object& get_fund_withdraw(  const account_name_type& owner, const string& fund_name, uint32_t request_id )const;
         const fund_withdraw_object* find_fund_withdraw( const account_name_type& owner, const string& fund_name, uint32_t request_id )const;

         const account_auth_object& get_auth_token(  const account_name_type& account, string authtype )const;
         const account_auth_object* find_auth_token( const account_name_type& account, string authtype )const;

         const dynamic_global_property_object&  get_dynamic_global_properties()const;
         const node_property_object&            get_node_properties()const;
         const bobserver_schedule_object&       get_bobserver_schedule_object()const;
         const hardfork_property_object&        get_hardfork_property_object()const;

         void max_bandwidth_per_share()const;

         /**
          *  Calculate the percent of block production slots that were missed in the
          *  past 128 blocks, not including the current block.
          */
         uint32_t bobserver_participation_rate()const;

         void                                   add_checkpoints( const flat_map<uint32_t,block_id_type>& checkpts );
         const flat_map<uint32_t,block_id_type> get_checkpoints()const { return _checkpoints; }
         bool                                   before_last_checkpoint()const;

         bool push_block( const signed_block& b, uint32_t skip = skip_nothing );
         void push_transaction( const signed_transaction& trx, uint32_t skip = skip_nothing );
         void _maybe_warn_multiple_production( uint32_t height )const;
         bool _push_block( const signed_block& b );
         void _push_transaction( const signed_transaction& trx );

         signed_block generate_block(
            const fc::time_point_sec when,
            const account_name_type& bobserver_owner,
            const fc::ecc::private_key& block_signing_private_key,
            uint32_t skip
            );
         signed_block _generate_block(
            const fc::time_point_sec when,
            const account_name_type& bobserver_owner,
            const fc::ecc::private_key& block_signing_private_key
            );

         void pop_block();
         void clear_pending();

         /**
          *  This method is used to track applied operations during the evaluation of a block, these
          *  operations should include any operation actually included in a transaction as well
          *  as any implied/virtual operations that resulted, such as filling an order.
          *  The applied operations are cleared after post_apply_operation.
          */
         void notify_pre_apply_operation( operation_notification& note );
         void notify_post_apply_operation( const operation_notification& note );
         void push_virtual_operation( const operation& op, bool force = false ); // vops are not needed for low mem. Force will push them on low mem.
         void notify_pre_apply_block( const signed_block& block );
         void notify_applied_block( const signed_block& block );
         void notify_on_pending_transaction( const signed_transaction& tx );
         void notify_on_pre_apply_transaction( const signed_transaction& tx );
         void notify_on_applied_transaction( const signed_transaction& tx );
         void notify_on_apply_hardfork( const uint32_t hardfork );

         /**
          *  This signal is emitted for plugins to process every operation after it has been fully applied.
          */
         fc::signal<void(const operation_notification&)> pre_apply_operation;
         fc::signal<void(const operation_notification&)> post_apply_operation;

         fc::signal<void(const signed_block&)>           pre_apply_block;

         /**
          *  This signal is emitted after all operations and virtual operation for a
          *  block have been applied but before the get_applied_operations() are cleared.
          *
          *  You may not yield from this callback because the blockchain is holding
          *  the write lock and may be in an "inconstant state" until after it is
          *  released.
          */
         fc::signal<void(const signed_block&)>           applied_block;

         /**
          * This signal is emitted any time a new transaction is added to the pending
          * block state.
          */
         fc::signal<void(const signed_transaction&)>     on_pending_transaction;

         /**
          * This signla is emitted any time a new transaction is about to be applied
          * to the chain state.
          */
         fc::signal<void(const signed_transaction&)>     on_pre_apply_transaction;

         /**
          * This signal is emitted any time a new transaction has been applied to the
          * chain state.
          */
         fc::signal<void(const signed_transaction&)>     on_applied_transaction;

         /**
          * This signal is emitted hardfork time
          */
         fc::signal< void( const uint32_t& ) > on_apply_hardfork;

         /**
          *  Emitted After a block has been applied and committed.  The callback
          *  should not yield and should execute quickly.
          */
         //fc::signal<void(const vector< graphene::db2::generic_id >&)> changed_objects;

         /** this signal is emitted any time an object is removed and contains a
          * pointer to the last value of every object that was removed.
          */
         //fc::signal<void(const vector<const object*>&)>  removed_objects;

         //////////////////// db_bobserver_schedule.cpp ////////////////////

         /**
          * @brief Get the bobserver scheduled for block production in a slot.
          *
          * slot_num always corresponds to a time in the future.
          *
          * If slot_num == 1, returns the next scheduled bobserver.
          * If slot_num == 2, returns the next scheduled bobserver after
          * 1 block gap.
          *
          * Use the get_slot_time() and get_slot_at_time() functions
          * to convert between slot_num and timestamp.
          *
          * Passing slot_num == 0 returns SIGMAENGINE_NULL_BOBSERVER
          */
         account_name_type get_scheduled_bobserver(uint32_t slot_num)const;

         /**
          * Get the time at which the given slot occurs.
          *
          * If slot_num == 0, return time_point_sec().
          *
          * If slot_num == N for N > 0, return the Nth next
          * block-interval-aligned time greater than head_block_time().
          */
         fc::time_point_sec get_slot_time(uint32_t slot_num)const;

         /**
          * Get the last slot which occurs AT or BEFORE the given time.
          *
          * The return value is the greatest value N such that
          * get_slot_time( N ) <= when.
          *
          * If no such N exists, return 0.
          */
         uint32_t get_slot_at_time(fc::time_point_sec when)const;
         
         void adjust_balance( const account_object& a, const asset& delta );
         void adjust_savings_balance( const account_object& a, const asset& delta );
         void adjust_fund_balance( const string name, const asset& delta );
         void adjust_fund_withdraw_balance( const string name, const asset& delta );
         void adjust_supply( const asset& delta );
         void update_owner_authority( const account_object& account, const authority& owner_authority );

         asset get_balance( const account_object& a, asset_symbol_type symbol )const;
         asset get_balance( const string& aname, asset_symbol_type symbol )const { return get_balance( get_account(aname), symbol ); }

         asset get_savings_balance( const account_object& a, asset_symbol_type symbol )const;
         
         /** this updates the vote of a single bobserver as a result of a vote being added or removed*/
         void adjust_bobserver_vote( const bobserver_object& obj, share_type delta );

         /** clears all vote records for a particular account but does not update the
          * bobserver vote totals.  Vote totals should be updated first via a call to
          * adjust_proxied_bobserver_votes( a, -a.bobserver_vote_weight() )
          */
         void clear_bobserver_votes( const account_object& a );
         void account_recovery_processing();

         time_point_sec   head_block_time()const;
         uint32_t         head_block_num()const;
         block_id_type    head_block_id()const;

         node_property_object& node_properties();

         uint32_t last_non_undoable_block_num() const;
         //////////////////// db_init.cpp ////////////////////

         void initialize_evaluators();
         void set_custom_operation_interpreter( const std::string& id, std::shared_ptr< custom_operation_interpreter > registry );
         std::shared_ptr< custom_operation_interpreter > get_custom_evaluator( const std::string& id );

         /// Reset the object graph in-memory
         void initialize_indexes();
         void init_schema();
         void init_genesis(uint64_t initial_supply = SIGMAENGINE_INIT_SUPPLY );

         /** when popping a block, the transactions that were removed get cached here so they
          * can be reapplied at the proper time */
         std::deque< signed_transaction >       _popped_tx;

         bool has_hardfork( uint32_t hardfork )const;
         
         /* For testing and debugging only. Given a hardfork
            with id N, applies all hardforks with id <= N */
         void set_hardfork( uint32_t hardfork, bool process_now = true );

         /**
          * get total supply
          * @Return total supply list of each coin
          * */
         std::map<asset_symbol_type, asset> get_total_supply()const;

         void validate_invariants()const;
         /**
          * @}
          */

         const std::string& get_json_schema() const;

         void set_flush_interval( uint32_t flush_blocks );
         void show_free_memory( bool force );
         // bool skip_transaction_delta_check = true;

         void process_funds();
         void process_savings_withdraws();
         void process_fund_withdraws();

         const common_fund_object&              get_common_fund( const string name )const;
         const dapp_reward_fund_object&         get_dapp_reward_fund()const;
         void adjust_dapp_reward_fund_balance( const asset& delta );

         void add_blacklist(const string account);
         bool is_blacklist(const account_name_type account);

   protected:
         //Mark pop_undo() as protected -- we do not want outside calling pop_undo(); it should call pop_block() instead
         //void pop_undo() { object_database::pop_undo(); }
         void notify_changed_objects();

      private:
         optional< chainbase::database::session > _pending_tx_session;

         void apply_block( const signed_block& next_block, uint32_t skip = skip_nothing );
         void apply_transaction( const signed_transaction& trx, uint32_t skip = skip_nothing );
         void _apply_block( const signed_block& next_block );
         void _apply_transaction( const signed_transaction& trx );
         void apply_operation( const operation& op );


         ///Steps involved in applying a new block
         ///@{

         const bobserver_object& validate_block_header( uint32_t skip, const signed_block& next_block )const;
         void create_block_summary(const signed_block& next_block);

         void clear_null_account_balance();

         void update_global_dynamic_data( const signed_block& b );
         void update_signing_bobserver(const bobserver_object& signing_bobserver, const signed_block& new_block);
         void update_last_irreversible_block();
         void clear_expired_transactions();
         void process_header_extensions( const signed_block& next_block );

         void init_hardforks();
         void process_hardforks();
         void apply_hardfork( uint32_t hardfork );

         ///@}

         std::unique_ptr< database_impl > _my;

         vector< signed_transaction >  _pending_tx;
         fork_database                 _fork_db;
         fc::time_point_sec            _hardfork_times[ SIGMAENGINE_NUM_HARDFORKS + 1 ];
         protocol::hardfork_version    _hardfork_versions[ SIGMAENGINE_NUM_HARDFORKS + 1 ];

         block_log                     _block_log;

         // this function needs access to _plugin_index_signal
         template< typename MultiIndexType >
         friend void add_plugin_index( database& db );

         fc::signal< void() >          _plugin_index_signal;

         transaction_id_type           _current_trx_id;
         uint32_t                      _current_block_num    = 0;
         uint16_t                      _current_trx_in_block = 0;
         uint16_t                      _current_op_in_trx    = 0;
         uint16_t                      _current_virtual_op   = 0;

         flat_map<uint32_t,block_id_type>  _checkpoints;

         node_property_object              _node_property_object;

         uint32_t                      _flush_blocks = 0;
         uint32_t                      _next_flush_block = 0;

         uint32_t                      _last_free_gb_printed = 0;

         flat_map< std::string, std::shared_ptr< custom_operation_interpreter > >   _custom_operation_interpreters;
         std::string                   _json_schema;

         vector<account_name_type>     _black_list;
   };

} }
