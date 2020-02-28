#ifndef LEDGER_APP_IOST_IOST_H
#define LEDGER_APP_IOST_IOST_H

#include <stdint.h>
#include <stddef.h>

#define IOST_NET_TYPE 44
#define IOST_COIN_ID 291

struct _Transaction;

// Forward declare to avoid including os.h in a header file
struct cx_ecfp_256_private_key_s;
struct cx_ecfp_256_public_key_s;

void iost_transaction_add_action(struct _Transaction *tx, const char *contract, const char *abi, const void *data);

extern int iost_derive_keypair(
    const uint32_t * const bip_32_path,
    const int bip_32_length,
    /* out */ struct cx_ecfp_256_private_key_s* secret_key,
    /* out */ struct cx_ecfp_256_public_key_s* public_key
);

extern void iost_sign(
    const uint32_t * const bip_32_path,
    const int bip_32_length,
    const uint8_t* tx,
    uint8_t tx_len,
    /* out */ uint8_t* result
);

extern void public_key_to_bytes(
    const struct cx_ecfp_256_public_key_s* public_key,
    /* out */ uint8_t* dst
);

//extern const char* iost_format_tinybar(const uint64_t tinybar);

#endif // LEDGER_APP_IOST_IOST_H
