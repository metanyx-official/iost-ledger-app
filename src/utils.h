#ifndef LEDGER_APP_IOST_UTILS_H
#define LEDGER_APP_IOST_UTILS_H

#include <stdint.h>

extern void bin2hex(uint8_t *dst, uint8_t *data, uint64_t inlen);
extern uint8_t set_error_code(uint8_t *buffer, uint16_t value, uint8_t offset);
extern void callback_os_exit(unsigned int exit_code);
extern const uint8_t* chars_2_bytes(const char* const chars);
extern const char* bytes_2_chars(const uint8_t* const bytes);

#endif // LEDGER_APP_IOST_UTILS_H
