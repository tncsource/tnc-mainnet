#pragma once

#include <sigmaengine/protocol/hardfork.hpp>

#define SIGMAENGINE_VERSION                                         ( version(0, 1, 258) )

#define SIGMAENGINE_BLOCKCHAIN_VERSION                              ( version(0, 1, 0) )
#define SIGMAENGINE_BLOCKCHAIN_HARDFORK_VERSION                     ( hardfork_version( SIGMAENGINE_BLOCKCHAIN_VERSION ) )

#define SIGMAENGINE_BLOCKCHAIN_PRECISION_DIGITS                     6

#define SIGMAENGINE_INIT_PUBLIC_KEY_STR                             "TNC8AC92qZFLfzvZ5hQtL4L7ZdmbEMkj3otccobDGcSrRQjSY4fhG" // 5Kbn1PrneRnSYob5rToDmDmkZg5ZLND6ZQKYUQj7WvkAZAnup1W
#define SIGMAENGINE_CHAIN_ID                                        (sigmaengine::protocol::chain_id_type())
#define SGT_SYMBOL                                                  (uint64_t(SIGMAENGINE_BLOCKCHAIN_PRECISION_DIGITS) | (uint64_t('T') << 8) | (uint64_t('N') << 16) | (uint64_t('C') << 24) )///< TNC with 8 digits of precision
#define SIGMAENGINE_ADDRESS_PREFIX                                  "TNC"
#define SIGMAENGINE_GENESIS_TIME                                    (fc::time_point_sec(1571116444))
#define SIGMAENGINE_OWNER_AUTH_RECOVERY_PERIOD                      fc::days(30)
#define SIGMAENGINE_ACCOUNT_RECOVERY_REQUEST_EXPIRATION_PERIOD      fc::days(1)
#define SIGMAENGINE_OWNER_UPDATE_LIMIT                              fc::seconds(5)
#define SIGMAENGINE_OWNER_AUTH_HISTORY_TRACKING_START_BLOCK_NUM     0

#define SIGMAENGINE_BLOCK_INTERVAL                                  3
#define SIGMAENGINE_BLOCKS_PER_YEAR                                 (365*24*60*60/SIGMAENGINE_BLOCK_INTERVAL)
#define SIGMAENGINE_BLOCKS_PER_DAY                                  (24*60*60/SIGMAENGINE_BLOCK_INTERVAL)
#define SIGMAENGINE_START_MINER_VOTING_BLOCK                        (SIGMAENGINE_BLOCKS_PER_DAY * 30)

#define SIGMAENGINE_INIT_MINER_NAME                                 "chainmaker"
#define SIGMAENGINE_NUM_INIT_MINERS                                 1
#define SIGMAENGINE_INIT_TIME                                       (fc::time_point_sec());

#define SIGMAENGINE_NUM_BOBSERVERS                                  21 

#define SIGMAENGINE_MAX_VOTED_BOBSERVERS_HF0                        17
#define SIGMAENGINE_MAX_MINER_BOBSERVERS_HF0                        4
#define SIGMAENGINE_MAX_RUNNER_BOBSERVERS_HF0                       10

#define SIGMAENGINE_HARDFORK_REQUIRED_BOBSERVERS                    17

#define SIGMAENGINE_MAX_TIME_UNTIL_EXPIRATION                       (60*60) // seconds,  aka: 1 hour
#define SIGMAENGINE_MAX_MEMO_SIZE                                   2048

#define SIGMAENGINE_100_PERCENT                                     10000
#define SIGMAENGINE_1_PERCENT                                       (SIGMAENGINE_100_PERCENT/100)
#define SIGMAENGINE_1_TENTH_PERCENT                                 (SIGMAENGINE_100_PERCENT/1000)

#define SIGMAENGINE_MAX_RESERVE_RATIO                               (20000)

#define SIGMAENGINE_CREATE_ACCOUNT_WITH_SIGMAENGINE_MODIFIER        30
#define SIGMAENGINE_CREATE_ACCOUNT_DELEGATION_RATIO                 5

#define SIGMAENGINE_MIN_ACCOUNT_NAME_LENGTH                         3
#define SIGMAENGINE_MAX_ACCOUNT_NAME_LENGTH                         64

#define SIGMAENGINE_MIN_PERMLINK_LENGTH                             0
#define SIGMAENGINE_MAX_PERMLINK_LENGTH                             256
#define SIGMAENGINE_MAX_BOBSERVER_URL_LENGTH                        2048

#define SIGMAENGINE_INIT_SUPPLY                                     int64_t(0)

#define SIGMAENGINE_MAX_SHARE_SUPPLY                                int64_t(1000000000000000ll)  
#define SIGMAENGINE_MAX_SIG_CHECK_DEPTH                             2

#define SIGMAENGINE_MAX_TRANSACTION_SIZE                            (1024*64*200)
#define SIGMAENGINE_MAX_BLOCK_SIZE                                  uint32_t( SIGMAENGINE_MAX_TRANSACTION_SIZE ) * uint32_t( SIGMAENGINE_BLOCK_INTERVAL * 2000 )
#define SIGMAENGINE_MIN_BLOCK_SIZE                                  115

#define SIGMAENGINE_MAX_UNDO_HISTORY                                10000

#define SIGMAENGINE_IRREVERSIBLE_THRESHOLD                          (75 * SIGMAENGINE_1_PERCENT)

#define SIGMAENGINE_MINER_ACCOUNT                                   "miners"
#define SIGMAENGINE_NULL_ACCOUNT                                    "null"
#define SIGMAENGINE_TEMP_ACCOUNT                                    "temp"
#define SIGMAENGINE_ROOT_ACCOUNT                                    "root"
#define SIGMAENGINE_ROOT_POST_PARENT                                (account_name_type())

#define SIGMAENGINE_DEPOSIT_FUND_NAME                               ("deposit")
#define SIGMAENGINE_MAX_STAKING_MONTH                               12
#define SIGMAENGINE_MAX_USER_TYPE                                   2

#define SIGMAENGINE_MAX_COMMENT_DEPTH                               0xffff // 64k
#define SIGMAENGINE_MIN_ROOT_COMMENT_INTERVAL                       (fc::seconds(3)) // 3 seconds // (fc::seconds(60*5)) // 5 minutes
#define SIGMAENGINE_MIN_REPLY_INTERVAL                              (fc::seconds(3)) // 3 seconds // (fc::seconds(20)) // 20 seconds

#define SIGMAENGINE_DAPP_TRANSACTION_FEE                            asset( 0, SGT_SYMBOL ) 
#define SIGMAENGINE_STAKING_INTEREST_PRECISION_DIGITS               3 
#define SIGMAENGINE_STAKING_INTEREST_PRECISION                      std::pow(10, SIGMAENGINE_STAKING_INTEREST_PRECISION_DIGITS )

#define SIGMAENGINE_TOKEN_MAX                                         int64_t(90000000000ll)

#define SIGMAENGINE_TRANSFER_SAVINGS_MIN_MONTH                      1
#define SIGMAENGINE_TRANSFER_SAVINGS_MAX_MONTH                      24

#ifdef IS_TEST_NET
   #define SIGMAENGINE_TRANSFER_SAVINGS_CYCLE                       (fc::seconds(30))
#else
   #define SIGMAENGINE_TRANSFER_SAVINGS_CYCLE                       (fc::days(30))
#endif

#ifdef IS_TEST_NET
#define SIGMAENGINE_HARDFORK_REQUIRED_BOBSERVERS                 1
#define SIGMAENGINE_HARDFORK_REQUIRED_BOBSERVERS_HF2             3
#else
#define SIGMAENGINE_HARDFORK_REQUIRED_BOBSERVERS                 17 // 17 of the 21 dpos bobservers (20 elected and 1 virtual time) required for hardfork. This guarantees 75% participation on all subsequent rounds.
#define SIGMAENGINE_HARDFORK_REQUIRED_BOBSERVERS_HF2             11 // 17/3 * 2
#endif

#ifdef IS_TEST_NET
#define SIGMAENGINE_MAX_FEED_AGE_SECONDS                         (10*60) // 10 min
#define SIGMAENGINE_MIN_FEEDS                                    3 //(FUTUREPIA_NUM_BOBSERVERS/3) /// protects the network from conversions before price has been established
#else
#define SIGMAENGINE_MAX_FEED_AGE_SECONDS                         (60*60*24*7) // 7 days
#define SIGMAENGINE_MIN_FEEDS                                    (SIGMAENGINE_NUM_BOBSERVERS/3) /// protects the network from conversions before price has been established
#endif

#define MAX_COMMENT_DEPTH                               0xffff // 64k
#define SOFT_MAX_COMMENT_DEPTH                          0xff // 255

#define SIGMA_DAPP
