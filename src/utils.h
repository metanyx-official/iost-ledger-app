#ifndef LEDGER_APP_IOST_UTILS_H
#define LEDGER_APP_IOST_UTILS_H

#include <stdint.h>
#include <os.h>

extern void public_key_to_bytes(uint8_t *dst, cx_ecfp_public_key_t *public);
extern void bin2hex(uint8_t *dst, uint8_t *data, uint64_t inlen);
extern uint8_t set_error_code(uint8_t *buffer, uint8_t offset, uint16_t value);


#endif // LEDGER_APP_IOST_UTILS_H
