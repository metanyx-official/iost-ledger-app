#ifndef LEDGER_APP_IOST_ERRORS_H
#define LEDGER_APP_IOST_ERRORS_H

// Based on ISO7816
typedef enum {
    EXCEPTION_WRONG_STATE                          = 0x6000,
    EXCEPTION_EXECUTION_ERROR                      = 0x6400,
    EXCEPTION_WRONG_LENGTH                         = 0x6700,
    EXCEPTION_EMPTY_BUFFER                         = 0x6982,
    EXCEPTION_OUTPUT_BUFFER_TOO_SMALL              = 0x6983,
    EXCEPTION_DATA_INVALID                         = 0x6984,
    EXCEPTION_USER_REJECTED                        = 0x6985, // User rejected action
    EXCEPTION_COMMAND_NOT_ALLOWED                  = 0x6986,
    EXCEPTION_BAD_KEY_HANDLE                       = 0x6A80,
    EXCEPTION_INVALID_P1P2                         = 0x6B00,
    EXCEPTION_INS_NOT_SUPPORTED                    = 0x6D00, // Instruction request is unknown
    EXCEPTION_CLA_NOT_SUPPORTED                    = 0x6E00, // APDU buffer is malformed
    EXCEPTION_INTERNAL_ERROR                       = 0x6F00,
    EXCEPTION_PROPRIETARY_CRYPTO_NOT_AVAILABLE     = 0x6F01,
    EXCEPTION_PROPRIETARY_USER_CANCELLED           = 0x6F02,
    EXCEPTION_PROPRIETARY_INVALID_PARAMETERS       = 0x6F03,
    EXCEPTION_PROPRIETARY_INVALID_DATA             = 0x6F04,
    EXCEPTION_PROPRIETARY_INTERNAL                 = 0x6FFF,
    EXCEPTION_OK                                   = 0x9000,
    EXCEPTION_BUSY                                 = 0x9001
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
