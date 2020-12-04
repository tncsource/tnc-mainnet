#pragma once
#include <sigmaengine/protocol/base.hpp>
#include <sigmaengine/protocol/block_header.hpp>
#include <sigmaengine/protocol/asset.hpp>

#include <fc/utf8.hpp>
#include <fc/crypto/equihash.hpp>

namespace sigmaengine { namespace protocol {

   inline void validate_account_name( const string& name )
   {
      FC_ASSERT( is_valid_account_name( name ), "Account name ${n} is invalid", ("n", name) );
   }

   inline void validate_permlink( const string& permlink )
   {
      FC_ASSERT( permlink.size() < SIGMAENGINE_MAX_PERMLINK_LENGTH, "permlink is too long" );
      FC_ASSERT( fc::is_utf8( permlink ), "permlink not formatted in UTF8" );
   }

   struct account_create_operation : public base_operation
   {
      account_name_type creator;
      account_name_type new_account_name;
      authority         owner;
      authority         active;
      authority         posting;
      public_key_type   memo_key;
      string            json_metadata;

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert(creator); }
   };

   struct account_update_operation : public base_operation
   {
      account_name_type             account;
      optional< authority >         owner;
      optional< authority >         active;
      optional< authority >         posting;
      public_key_type               memo_key;
      string                        json_metadata;

      void validate()const;

      void get_required_owner_authorities( flat_set<account_name_type>& a )const
      { if( owner ) a.insert( account ); }

      void get_required_active_authorities( flat_set<account_name_type>& a )const
      { if( !owner ) a.insert( account ); }
   };

   /**
    * @ingroup operations
    *
    * @brief Transfers PIA from one account to another.
    */
   struct transfer_operation : public base_operation
   {
      account_name_type from;
      /// Account to transfer asset to
      account_name_type to;
      /// The amount of asset to transfer from @ref from to @ref to
      asset             amount;

      /// The memo is plain-text, any encryption on the memo is up to
      /// a higher level protocol.
      string            memo;

      void              validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert(from); }
   };

   /**
    *  Users who wish to become a bobserver must pay a fee acceptable to
    *  the current bobservers to apply for the position and allow voting
    *  to begin.
    *
    *  If the owner isn't a bobserver they will become a bobserver.  Bobservers
    *  are charged a fee equal to 1 weeks worth of bobserver pay which in
    *  turn is derived from the current share supply.  The fee is
    *  only applied if the owner is not already a bobserver.
    *
    *  If the block_signing_key is null then the bobserver is removed from
    *  contention.  The network will pick the top 21 bobservers for
    *  producing blocks.
    */
   struct bobserver_update_operation : public base_operation
   {
      account_name_type root;
      account_name_type owner;
      string            url;
      public_key_type   block_signing_key;

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( root ); }
   };

   /**
    * @brief provides a generic way to add higher level protocols on top of bobserver consensus
    * @ingroup operations
    *
    * There is no validation for this operation other than that required auths are valid
    */
   struct custom_operation : public base_operation
   {
      flat_set< account_name_type > required_auths;
      uint16_t                      id = 0;
      vector< char >                data;

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ for( const auto& i : required_auths ) a.insert(i); }
   };


   /** serves the same purpose as custom_operation but also supports required posting authorities. Unlike custom_operation,
    * this operation is designed to be human readable/developer friendly.
    **/
   struct custom_json_operation : public base_operation
   {
      flat_set< account_name_type > required_auths;
      flat_set< account_name_type > required_posting_auths;
      string                        id; ///< must be less than 32 characters long
      string                        json; ///< must be proper utf8 / JSON string.

      void validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ for( const auto& i : required_auths ) a.insert(i); }
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ for( const auto& i : required_posting_auths ) a.insert(i); }
   };

   struct custom_json_dapp_operation : public base_operation
   {
      flat_set< account_name_type > required_owner_auths;
      flat_set< account_name_type > required_active_auths;
      flat_set< account_name_type > required_posting_auths;
      vector< authority >           required_auths;
      string                        id; ///< must be less than 32 characters long
      string                        json; ///< must be proper utf8 / JSON string.

      void validate()const;
      void get_required_owner_authorities( flat_set<account_name_type>& a )const{ for( const auto& i : required_owner_auths ) a.insert(i); }
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ for( const auto& i : required_active_auths ) a.insert(i); }
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ for( const auto& i : required_posting_auths ) a.insert(i); }
      void get_required_authorities( vector< authority >& a )const{ for( const auto& i : required_auths ) a.push_back( i ); }
   };

   struct custom_binary_operation : public base_operation
   {
      flat_set< account_name_type > required_owner_auths;
      flat_set< account_name_type > required_active_auths;
      flat_set< account_name_type > required_posting_auths;
      vector< authority >           required_auths;

      string                        id; ///< must be less than 32 characters long
      vector< char >                data;

      void validate()const;
      void get_required_owner_authorities( flat_set<account_name_type>& a )const{ for( const auto& i : required_owner_auths ) a.insert(i); }
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ for( const auto& i : required_active_auths ) a.insert(i); }
      void get_required_posting_authorities( flat_set<account_name_type>& a )const{ for( const auto& i : required_posting_auths ) a.insert(i); }
      void get_required_authorities( vector< authority >& a )const{ for( const auto& i : required_auths ) a.push_back( i ); }
   };

   /**
    * All account recovery requests come from a listed recovery account. This
    * is secure based on the assumption that only a trusted account should be
    * a recovery account. It is the responsibility of the recovery account to
    * verify the identity of the account holder of the account to recover by
    * whichever means they have agreed upon. The blockchain assumes identity
    * has been verified when this operation is broadcast.
    *
    * This operation creates an account recovery request which the account to
    * recover has 24 hours to respond to before the request expires and is
    * invalidated.
    *
    * There can only be one active recovery request per account at any one time.
    * Pushing this operation for an account to recover when it already has
    * an active request will either update the request to a new new owner authority
    * and extend the request expiration to 24 hours from the current head block
    * time or it will delete the request. To cancel a request, simply set the
    * weight threshold of the new owner authority to 0, making it an open authority.
    *
    * Additionally, the new owner authority must be satisfiable. In other words,
    * the sum of the key weights must be greater than or equal to the weight
    * threshold.
    *
    * This operation only needs to be signed by the the recovery account.
    * The account to recover confirms its identity to the blockchain in
    * the recover account operation.
    */
   struct request_account_recovery_operation : public base_operation
   {
      account_name_type recovery_account;       ///< The recovery account is listed as the recovery account on the account to recover.

      account_name_type account_to_recover;     ///< The account to recover. This is likely due to a compromised owner authority.

      authority         new_owner_authority;    ///< The new owner authority the account to recover wishes to have. This is secret
                                                ///< known by the account to recover and will be confirmed in a recover_account_operation

      extensions_type   extensions;             ///< Extensions. Not currently used.

      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( recovery_account ); }

      void validate() const;
   };


   /**
    * Recover an account to a new authority using a previous authority and verification
    * of the recovery account as proof of identity. This operation can only succeed
    * if there was a recovery request sent by the account's recover account.
    *
    * In order to recover the account, the account holder must provide proof
    * of past ownership and proof of identity to the recovery account. Being able
    * to satisfy an owner authority that was used in the past 30 days is sufficient
    * to prove past ownership. The get_owner_history function in the database API
    * returns past owner authorities that are valid for account recovery.
    *
    * Proving identity is an off chain contract between the account holder and
    * the recovery account. The recovery request contains a new authority which
    * must be satisfied by the account holder to regain control. The actual process
    * of verifying authority may become complicated, but that is an application
    * level concern, not a blockchain concern.
    *
    * This operation requires both the past and future owner authorities in the
    * operation because neither of them can be derived from the current chain state.
    * The operation must be signed by keys that satisfy both the new owner authority
    * and the recent owner authority. Failing either fails the operation entirely.
    *
    * If a recovery request was made inadvertantly, the account holder should
    * contact the recovery account to have the request deleted.
    *
    * The two setp combination of the account recovery request and recover is
    * safe because the recovery account never has access to secrets of the account
    * to recover. They simply act as an on chain endorsement of off chain identity.
    * In other systems, a fork would be required to enforce such off chain state.
    * Additionally, an account cannot be permanently recovered to the wrong account.
    * While any owner authority from the past 30 days can be used, including a compromised
    * authority, the account can be continually recovered until the recovery account
    * is confident a combination of uncompromised authorities were used to
    * recover the account. The actual process of verifying authority may become
    * complicated, but that is an application level concern, not the blockchain's
    * concern.
    */
   struct recover_account_operation : public base_operation
   {
      account_name_type account_to_recover;        ///< The account to be recovered

      authority         new_owner_authority;       ///< The new owner authority as specified in the request account recovery operation.

      authority         recent_owner_authority;    ///< A previous owner authority that the account holder will use to prove past ownership of the account to be recovered.

      extensions_type   extensions;                ///< Extensions. Not currently used.

      void get_required_authorities( vector< authority >& a )const
      {
         a.push_back( new_owner_authority );
         a.push_back( recent_owner_authority );
      }

      void validate() const;
   };


   /**
    *  This operation allows recovery_accoutn to change account_to_reset's owner authority to
    *  new_owner_authority after 60 days of inactivity.
    */
   struct reset_account_operation : public base_operation {
      account_name_type reset_account;
      account_name_type account_to_reset;
      authority         new_owner_authority;

      void get_required_active_authorities( flat_set<account_name_type>& a )const { a.insert( reset_account ); }
      void validate()const;
   };

   /**
    * This operation allows 'account' owner to control which account has the power
    * to execute the 'reset_account_operation' after 60 days.
    */
   struct set_reset_account_operation : public base_operation {
      account_name_type account;
      account_name_type current_reset_account;
      account_name_type reset_account;
      void validate()const;
      void get_required_owner_authorities( flat_set<account_name_type>& a )const
      {
         if( current_reset_account.size() )
            a.insert( account );
      }

      void get_required_posting_authorities( flat_set<account_name_type>& a )const
      {
         if( !current_reset_account.size() )
            a.insert( account );
      }
   };


   /**
    * Each account lists another account as their recovery account.
    * The recovery account has the ability to create account_recovery_requests
    * for the account to recover. An account can change their recovery account
    * at any time with a 30 day delay. This delay is to prevent
    * an attacker from changing the recovery account to a malicious account
    * during an attack. These 30 days match the 30 days that an
    * owner authority is valid for recovery purposes.
    *
    * On account creation the recovery account is set either to the creator of
    * the account (The account that pays the creation fee and is a signer on the transaction)
    * or to the empty string if the account was mined. An account with no recovery
    * has the top voted bobserver as a recovery account, at the time the recover
    * request is created. Note: This does mean the effective recovery account
    * of an account with no listed recovery account can change at any time as
    * bobserver vote weights. The top voted bobserver is explicitly the most trusted
    * bobserver according to stake.
    */
   struct change_recovery_account_operation : public base_operation
   {
      account_name_type account_to_recover;     ///< The account that would be recovered in case of compromise
      account_name_type new_recovery_account;   ///< The account that creates the recover request
      extensions_type   extensions;             ///< Extensions. Not currently used.

      void get_required_owner_authorities( flat_set<account_name_type>& a )const{ a.insert( account_to_recover ); }
      void validate() const;
   };

   struct decline_voting_rights_operation : public base_operation
   {
      account_name_type account;
      bool              decline = true;

      void get_required_owner_authorities( flat_set<account_name_type>& a )const{ a.insert( account ); }
      void validate() const;
   };

   struct update_bproducer_operation : public base_operation
   {
      account_name_type root;
      account_name_type bobserver;
      bool              approve = true;

      void validate() const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert(root); }
   };

   struct except_bobserver_operation : public base_operation
   {
      account_name_type root;
      account_name_type bobserver;

      void validate() const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert(root); }
   };

   struct account_auth_operation : public base_operation
   {
      account_name_type account;
      string            auth_type;
      string            auth_token;

      void validate() const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert(account); }
   };

   struct print_operation : public base_operation
   {
      account_name_type root;
      account_name_type account;
      asset             amount;

      void  validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( root ); }
   };

   struct burn_operation : public base_operation
   {
      account_name_type root;
      account_name_type account;
      asset             amount;

      void  validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( root ); }
   };

   struct transfer_savings_operation : public base_operation {
      account_name_type from;
      uint32_t          request_id = 0;
      account_name_type to;
      asset             amount;
      asset             total_amount;
      uint8_t           split_pay_order = 0;
      uint8_t           split_pay_month = 0;
      string            memo;
      time_point_sec    complete;

      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( from ); }
      void validate() const;
   };

   struct cancel_transfer_savings_operation : public base_operation {
      account_name_type from;
      account_name_type to;
      asset             amount = asset( 0, SGT_SYMBOL );
      uint32_t          request_id = 0;

      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( from ); }
      void validate() const;
   };

   struct conclusion_transfer_savings_operation : public base_operation {
      account_name_type from;
      account_name_type to;
      uint32_t          request_id = 0;

      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( from ); }
      void validate() const;
   };

   struct staking_fund_operation : public base_operation {
      account_name_type from;
      string            fund_name;
      uint32_t          request_id = 0;
      asset             amount;
      string            memo;
      uint8_t           usertype = 0;
      uint8_t           month = 0;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( from ); }
      void validate() const;
   };

   struct conclusion_staking_operation : public base_operation {
      account_name_type root;
      account_name_type from;
      string            fund_name;
      uint32_t          request_id = 0;

      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( root ); }
      void validate() const;
   };

   struct return_staking_fund_operation : public base_operation {
      account_name_type root;
      string            fund_name;
      uint32_t          request_id = 0;
      account_name_type to;

      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( root ); }
      void validate() const;
   };

   struct transfer_fund_operation : public base_operation
   {
      account_name_type from;
      string            fund_name;
      asset             amount;
      string            memo;
      void              validate()const;

      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( from ); }
   };

   struct set_fund_interest_operation : public base_operation
   {
      account_name_type root;
      string            fund_name;
      uint8_t           usertype = 0;
      uint8_t           month = 0;
      string            percent_interest;
      void              validate()const;
      void get_required_active_authorities( flat_set<account_name_type>& a )const{ a.insert( root ); }
   };

   
} } // sigmaengine::protocol


FC_REFLECT( sigmaengine::protocol::transfer_savings_operation, (from)(request_id)(to)(amount)(total_amount)(split_pay_order)(split_pay_month)(memo)(complete) )
FC_REFLECT( sigmaengine::protocol::cancel_transfer_savings_operation, (from)(to)(amount)(request_id) )
FC_REFLECT( sigmaengine::protocol::conclusion_transfer_savings_operation, (from)(to)(request_id) )
FC_REFLECT( sigmaengine::protocol::staking_fund_operation, (from)(fund_name)(request_id)(amount)(memo)(usertype)(month) )
FC_REFLECT( sigmaengine::protocol::conclusion_staking_operation, (root)(from)(fund_name)(request_id) )
FC_REFLECT( sigmaengine::protocol::transfer_fund_operation, (from)(fund_name)(amount)(memo) )
FC_REFLECT( sigmaengine::protocol::set_fund_interest_operation, (root)(fund_name)(usertype)(month)(percent_interest) )
FC_REFLECT( sigmaengine::protocol::return_staking_fund_operation, (root)(fund_name)(request_id)(to) )


FC_REFLECT( sigmaengine::protocol::reset_account_operation, (reset_account)(account_to_reset)(new_owner_authority) )
FC_REFLECT( sigmaengine::protocol::set_reset_account_operation, (account)(current_reset_account)(reset_account) )

FC_REFLECT( sigmaengine::protocol::account_create_operation,
            (creator)
            (new_account_name)
            (owner)
            (active)
            (posting)
            (memo_key)
            (json_metadata) )

FC_REFLECT( sigmaengine::protocol::account_update_operation,
            (account)
            (owner)
            (active)
            (posting)
            (memo_key)
            (json_metadata) )

FC_REFLECT( sigmaengine::protocol::transfer_operation, (from)(to)(amount)(memo) )
FC_REFLECT( sigmaengine::protocol::bobserver_update_operation, (root)(owner)(url)(block_signing_key) )
FC_REFLECT( sigmaengine::protocol::custom_operation, (required_auths)(id)(data) )
FC_REFLECT( sigmaengine::protocol::custom_json_operation, (required_auths)(required_posting_auths)(id)(json) )
FC_REFLECT( sigmaengine::protocol::custom_json_dapp_operation, (required_owner_auths)(required_active_auths)(required_posting_auths)(required_auths)(id)(json) )
FC_REFLECT( sigmaengine::protocol::custom_binary_operation, (required_owner_auths)(required_active_auths)(required_posting_auths)(required_auths)(id)(data) )
FC_REFLECT( sigmaengine::protocol::request_account_recovery_operation, (recovery_account)(account_to_recover)(new_owner_authority)(extensions) );
FC_REFLECT( sigmaengine::protocol::recover_account_operation, (account_to_recover)(new_owner_authority)(recent_owner_authority)(extensions) );
FC_REFLECT( sigmaengine::protocol::change_recovery_account_operation, (account_to_recover)(new_recovery_account)(extensions) );
FC_REFLECT( sigmaengine::protocol::decline_voting_rights_operation, (account)(decline) );
FC_REFLECT( sigmaengine::protocol::update_bproducer_operation, (root)(bobserver)(approve) )
FC_REFLECT( sigmaengine::protocol::except_bobserver_operation, (root)(bobserver) )
FC_REFLECT( sigmaengine::protocol::account_auth_operation, (account)(auth_type)(auth_token) )
FC_REFLECT( sigmaengine::protocol::print_operation, (root)(account)(amount) )
FC_REFLECT( sigmaengine::protocol::burn_operation, (root)(account)(amount) )