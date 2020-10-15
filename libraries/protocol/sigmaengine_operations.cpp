#include <sigmaengine/protocol/sigmaengine_operations.hpp>
#include <fc/io/json.hpp>

#include <locale>

namespace sigmaengine { namespace protocol {

   bool inline is_asset_type( asset as, asset_symbol_type symbol )
   {
      return as.symbol == symbol;
   }

   void account_create_operation::validate() const
   {
      validate_account_name( new_account_name );
      owner.validate();
      active.validate();

      if ( json_metadata.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8(json_metadata), "JSON Metadata not formatted in UTF8" );
         FC_ASSERT( fc::json::is_valid(json_metadata), "JSON Metadata not valid JSON" );
      }
   }

   void account_update_operation::validate() const
   {
      validate_account_name( account );

      if ( json_metadata.size() > 0 )
      {
         FC_ASSERT( fc::is_utf8(json_metadata), "JSON Metadata not formatted in UTF8" );
         FC_ASSERT( fc::json::is_valid(json_metadata), "JSON Metadata not valid JSON" );
      }
   }

   void transfer_operation::validate() const
   { try {
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( amount.amount > 0, "Cannot transfer a negative amount (aka: stealing)" );
      FC_ASSERT( memo.size() < SIGMAENGINE_MAX_MEMO_SIZE, "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ), "Memo is not UTF8" );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }


   void custom_operation::validate() const {
      /// required auth accounts are the ones whose bandwidth is consumed
      FC_ASSERT( required_auths.size() > 0, "at least on account must be specified" );
   }

   void custom_json_operation::validate() const {
      FC_ASSERT( (required_auths.size() + required_posting_auths.size()) > 0, "at least on account must be specified" );
      FC_ASSERT( id.size() <= 32, "id is too long" );
      FC_ASSERT( fc::is_utf8(json), "JSON Metadata not formatted in UTF8" );
      FC_ASSERT( fc::json::is_valid(json), "JSON Metadata not valid JSON" );
   }

   void custom_json_dapp_operation::validate() const {
      /// required auth accounts are the ones whose bandwidth is consumed
      FC_ASSERT( ( required_owner_auths.size() + required_active_auths.size() + required_posting_auths.size()  + required_auths.size() ) > 0
         , "at least on account must be specified" );
      for( const auto& a : required_auths ) a.validate();
      FC_ASSERT( id.size() <= 32, "id is too long" );
      FC_ASSERT( fc::is_utf8(json), "JSON Metadata not formatted in UTF8" );
      FC_ASSERT( fc::json::is_valid(json), "JSON Metadata not valid JSON" );
   }

   void custom_binary_operation::validate() const {
      FC_ASSERT( ( required_owner_auths.size() + required_active_auths.size() + required_posting_auths.size() + required_auths.size() ) > 0
         , "at least on account must be specified" );
      FC_ASSERT( id.size() <= 32, "id is too long" );
      for( const auto& a : required_auths ) a.validate();
   }

   void request_account_recovery_operation::validate()const
   {
      validate_account_name( recovery_account );
      validate_account_name( account_to_recover );
      new_owner_authority.validate();
   }

   void recover_account_operation::validate()const
   {
      validate_account_name( account_to_recover );
      FC_ASSERT( !( new_owner_authority == recent_owner_authority ), "Cannot set new owner authority to the recent owner authority" );
      FC_ASSERT( !new_owner_authority.is_impossible(), "new owner authority cannot be impossible" );
      FC_ASSERT( !recent_owner_authority.is_impossible(), "recent owner authority cannot be impossible" );
      FC_ASSERT( new_owner_authority.weight_threshold, "new owner authority cannot be trivial" );
      new_owner_authority.validate();
      recent_owner_authority.validate();
   }

   void change_recovery_account_operation::validate()const
   {
      validate_account_name( account_to_recover );
      validate_account_name( new_recovery_account );
   }

   void decline_voting_rights_operation::validate()const
   {
      validate_account_name( account );
   }

   void reset_account_operation::validate()const
   {
      validate_account_name( reset_account );
      validate_account_name( account_to_reset );
      FC_ASSERT( !new_owner_authority.is_impossible(), "new owner authority cannot be impossible" );
      FC_ASSERT( new_owner_authority.weight_threshold, "new owner authority cannot be trivial" );
      new_owner_authority.validate();
   }

   void set_reset_account_operation::validate()const
   {
      validate_account_name( account );
      if( current_reset_account.size() )
         validate_account_name( current_reset_account );
      validate_account_name( reset_account );
      FC_ASSERT( current_reset_account != reset_account, "new reset account cannot be current reset account" );
   }

   void account_auth_operation::validate() const
   {
      validate_account_name( account );
   }

   void print_operation::validate() const
   {
      validate_account_name( account );
      FC_ASSERT( amount.amount > 0, "amount cannot be negative" );
   }

   void burn_operation::validate() const
   {
      validate_account_name( account );
      FC_ASSERT( amount.amount > 0, "amount cannot be negative" );
   }

   void transfer_savings_operation::validate()const {
      validate_account_name( from );
      validate_account_name( to );
      FC_ASSERT( amount.amount > 0 );
      FC_ASSERT( amount.symbol == SGT_SYMBOL );
      FC_ASSERT( request_id >= 0 );
      FC_ASSERT( memo.size() < SIGMAENGINE_MAX_MEMO_SIZE, "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ), "Memo is not UTF8" );
   }

   void cancel_transfer_savings_operation::validate()const {
      validate_account_name( from );
   }
   
   void conclusion_transfer_savings_operation::validate()const
   {
      validate_account_name( from );
   }

   void staking_fund_operation::validate()const {
      validate_account_name( from );
      FC_ASSERT( fund_name.size() > 0 );
      FC_ASSERT( request_id >= 0 );
      FC_ASSERT( amount.amount > 0 );
      FC_ASSERT( amount.symbol == SGT_SYMBOL );
      FC_ASSERT( usertype >= 0 && usertype <= 1 );
      FC_ASSERT( month >= 1 && month <= 12 );
      FC_ASSERT( memo.size() < SIGMAENGINE_MAX_MEMO_SIZE, "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ), "Memo is not UTF8" );
   }

   void conclusion_staking_operation::validate()const
   {
      validate_account_name( from );
      FC_ASSERT( fund_name.size() > 0 );
   }

   void return_staking_fund_operation::validate()const
   {
      validate_account_name( to );
      FC_ASSERT( fund_name.size() > 0 );
   }

   void transfer_fund_operation::validate() const
   { try {
      validate_account_name( from );
      FC_ASSERT( fund_name.size() >0 );
      FC_ASSERT( amount.amount > 0, "Cannot transfer a negative amount (aka: stealing)" );
      FC_ASSERT( amount.symbol == SGT_SYMBOL );
      FC_ASSERT( memo.size() < SIGMAENGINE_MAX_MEMO_SIZE, "Memo is too large" );
      FC_ASSERT( fc::is_utf8( memo ), "Memo is not UTF8" );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }

   void set_fund_interest_operation::validate() const
   { try {
      validate_account_name( root );
      FC_ASSERT( fund_name.size() > 0 );
      FC_ASSERT( usertype >= 0 && usertype <= 1 );
      FC_ASSERT( month >= 1 && month <= 12 );
      FC_ASSERT( percent_interest.size() > 0 );
   } FC_CAPTURE_AND_RETHROW( (*this) ) }
} } // sigmaengine::protocol
