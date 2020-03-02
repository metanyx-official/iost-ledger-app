#ifndef LEDGER_APP_IOST_BASE58_H
#define LEDGER_APP_IOST_BASE58_H

extern unsigned int encode_base_58(
    const void* const in,
    const unsigned int in_len,
    unsigned char* out
);

#endif // LEDGER_APP_IOST_BASE_H
