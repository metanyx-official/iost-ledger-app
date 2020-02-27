#ifndef LEDGER_APP_IOST_IO_H
#define LEDGER_APP_IOST_IO_H

#include <os.h>
#include <os_io_seproxyhal.h>
#include <stdint.h>

/* Although SEP-0005 only allows 3 bip32 path elements we support more */
#define APDU_MIN_SIZE 1
#define APDU_MAX_SIZE 512
#define BIP32_PATH_MASK 0x80000000
#define BIP32_PATH_LENGTH 3
#define BIP32_KEY_SIZE 32
//#define FULL_ADDRESS_LENGTH 54
//#define MAX_TX_SIZE 512

extern void io_exchange_with_code(uint16_t code, uint16_t tx);
extern int io_read_bip32(const uint8_t *dataBuffer, size_t size, uint32_t *bip32);

#endif // LEDGER_APP_IOST_IO_H
