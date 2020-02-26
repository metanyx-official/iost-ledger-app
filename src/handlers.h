#ifndef LEDGER_APP_IOST_HANDLERS_H
#define LEDGER_APP_IOST_HANDLERS_H

#include <stddef.h>
#include <stdint.h>

#define CLA 0xE0

// These are the offsets of various parts of a request APDU packet.
// INS identifies the commands.
// P1 and P2 are parameters to the command.
#define OFFSET_CLA 0
#define OFFSET_INS 1
#define OFFSET_P1 2
#define OFFSET_P2 3
#define OFFSET_LC 4
#define OFFSET_CDATA 5

/* Although SEP-0005 only allows 3 bip32 path elements we support more */
#define MAX_BIP32_LEN 10
#define MIN_APDU_SIZE 5

// CLA <INS> <-- Command Line Argument <Instruction>
#define INS_GET_APP_CONFIGURATION 0x06
#define INS_SIGN_TRANSACTION 0x04
#define INS_GET_PUBLIC_KEY 0x02
#define P1_CONFIRM 0x00
#define P1_NO_CONFIRM 0x01
#define P2_DISPLAY_PUBKEY 0x00
#define P2_DISPLAY_ADDRESS 0x01
#define P1_FIRST 0x00
#define P1_MORE 0x80
#define P2_LAST 0x00
#define P2_MORE 0x80

typedef void handler_fn_t(
    uint8_t p1,
    uint8_t p2,
    uint8_t* buffer,
    uint16_t len,
    /* out */ volatile unsigned int* flags,
    /* out */ volatile unsigned int* tx
);

extern handler_fn_t handle_get_app_configuration;
extern handler_fn_t handle_get_public_key;
extern handler_fn_t handle_sign_transaction;

#endif // LEDGER_APP_IOST_HANDLERS_H
