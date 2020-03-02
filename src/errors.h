#ifndef LEDGER_APP_IOST_ERRORS_H
#define LEDGER_APP_IOST_ERRORS_H

// Based on ISO7816
typedef enum {

    SW_WRONG_STATE                  = 0x6000,
    SW_EXECUTION_ERROR              = 0x6400,
    SW_WRONG_LENGTH                 = 0x6700,
    SW_DATA_INVALID                 = 0x6984,
    SW_USER_REJECTED                = 0x6985,
    SW_COMMAND_NOT_ALLOWED          = 0x6986,
    SW_BAD_KEY_HANDLE               = 0x6A80,
    SW_INVALID_P1P2                 = 0x6B00,
    SW_INS_NOT_SUPPORTED            = 0x6D00,
    SW_CLA_NOT_SUPPORTED            = 0x6E00,
    SW_INTERNAL_ERROR               = 0x6F00,
    SW_OK                           = 0x9000
} status_word_t;

//// APDU buffer is malformed
//#define EXCEPTION_MALFORMED_APDU 0x6E00
//// Instruction request is unknown
//#define EXCEPTION_UNKNOWN_INS 0x6D00
//// User rejected action
//#define EXCEPTION_USER_REJECTED 0x6985
//// Ok
//#define EXCEPTION_OK 0x9000

#endif // LEDGER_APP_IOST_ERRORS_H
