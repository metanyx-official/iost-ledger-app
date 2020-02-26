#ifndef LEDGER_APP_IOST_IO_H
#define LEDGER_APP_IOST_IO_H

#include <os.h>
#include <os_io_seproxyhal.h>
#include <stdint.h>

extern void io_exchange_with_code(uint16_t code, uint16_t tx);
extern int io_read_bip32(const uint8_t *dataBuffer, size_t size, uint32_t *bip32);

#endif // LEDGER_APP_IOST_IO_H
