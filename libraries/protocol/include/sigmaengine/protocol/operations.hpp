#pragma once

#include <sigmaengine/protocol/operation_util.hpp>
#include <sigmaengine/protocol/sigmaengine_operations.hpp>
#include <sigmaengine/protocol/sigmaengine_virtual_operations.hpp>

namespace sigmaengine { namespace protocol {

   /** NOTE: do not change the order of any operations prior to the virtual operations
    * or it will trigger a hardfork.
    */
   typedef fc::static_variant<
            transfer_operation,
            account_create_operation,
            account_update_operation,
            bobserver_update_operation,
            custom_operation,
            custom_json_operation,
            request_account_recovery_operation,
            recover_account_operation,
            change_recovery_account_operation,
            custom_binary_operation,
            decline_voting_rights_operation,
            reset_account_operation,
            set_reset_account_operation,
            update_bproducer_operation,
            except_bobserver_operation,            
            account_auth_operation,
            
            print_operation,
            burn_operation,

            transfer_savings_operation,
            cancel_transfer_savings_operation,
            conclusion_transfer_savings_operation,
            staking_fund_operation,
            conclusion_staking_operation,
            transfer_fund_operation,
            set_fund_interest_operation,
            return_staking_fund_operation,

            /// virtual operations below this point           
            shutdown_bobserver_operation,
            hardfork_operation,
            
            dapp_fee_virtual_operation,
            dapp_reward_virtual_operation,

            fill_token_staking_fund_operation,
            fill_transfer_token_savings_operation,

            fill_staking_fund_operation,
            fill_transfer_savings_operation,

            custom_json_dapp_operation
            
         > operation;

   bool is_virtual_operation( const operation& op );
   string get_op_name( const operation& op );

} } // sigmaengine::protocol

DECLARE_OPERATION_TYPE( sigmaengine::protocol::operation )
FC_REFLECT_TYPENAME( sigmaengine::protocol::operation )
