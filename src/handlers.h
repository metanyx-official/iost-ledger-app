#ifndef LEDGER_APP_IOST_HANDLERS_H
#define LEDGER_APP_IOST_HANDLERS_H

#include <stddef.h>
#include <stdint.h>

typedef void handler_fn_t(
    const uint8_t p1,
    const uint8_t p2,
    const uint8_t* const buffer,
    const uint16_t buffer_length,
    /* out */ volatile uint8_t* flags,
    /* out */ volatile uint16_t* tx
);

extern handler_fn_t handle_get_configuration;
extern handler_fn_t handle_get_public_key;
extern handler_fn_t handle_sign_message;

extern void clear_context_get_configuration();
extern void clear_context_get_public_key();
extern void clear_context_sign_message();

#endif // LEDGER_APP_IOST_HANDLERS_H
