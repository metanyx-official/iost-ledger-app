#ifndef LEDGER_APP_IOST_IOST_H
#define LEDGER_APP_IOST_IOST_H

#include <stdint.h>

struct _Transaction;

void iost_transaction_add_action(struct _Transaction *tx, const char *contract, const char *abi, const void *data);


// Forward declare to avoid including os.h in a header file
struct cx_ecfp_256_public_key_s;
struct cx_ecfp_256_private_key_s;

extern void iost_derive_keypair(
    uint32_t index,
    /* out */ struct cx_ecfp_256_private_key_s* secret, 
    /* out */ struct cx_ecfp_256_public_key_s* public
);

extern void iost_sign(
    uint32_t index,
    const uint8_t* tx,
    uint8_t tx_len,
    /* out */ uint8_t* result
);

extern char* iost_format_tinybar(uint64_t tinybar);

#endif // LEDGER_APP_IOST_IOST_H
