#ifndef LEDGER_APP_IOST_HANDLERS_H
#define LEDGER_APP_IOST_HANDLERS_H

#include <stddef.h>
#include <stdint.h>

typedef void handler_fn_t(
    uint8_t p1,
    uint8_t p2,
    const uint8_t* const buffer,
    uint16_t size,
    /* out */ volatile unsigned int* flags,
    /* out */ volatile unsigned int* tx
);

extern handler_fn_t handle_get_configuration;
extern handler_fn_t handle_get_public_key;
extern handler_fn_t handle_sign_transaction;

#endif // LEDGER_APP_IOST_HANDLERS_H
