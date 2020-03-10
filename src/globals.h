#ifndef LEDGER_APP_IOST_GLOBALS_H
#define LEDGER_APP_IOST_GLOBALS_H

#define CLA 0xE0

// These are the offsets of various parts of a request APDU packet.
// INS identifies the commands.
// P1 and P2 are parameters to the command.
#define OFFSET_CLA 0
#define OFFSET_INS 1
#define OFFSET_P1 2
#define OFFSET_P2 3
#define OFFSET_LC 4
#define OFFSET_CDATA 5

// CLA <INS> <-- Command Line Argument <Instruction>
#define INS_RETURN_TO_DASHBOARD 0xFF
#define INS_SIGN_MESSAGE 0x08
#define INS_GET_PUBLIC_KEY 0x04
#define INS_GET_CONFIGURATION 0x02
#define INS_ECHO 0x01
#define INS_RESET 0x00
#define P1_CONFIRM 0x00
#define P1_SILENT 0x01
#define P2_BIN 0x00
#define P2_HEX 0x01
#define P2_BASE58 0x02
#define P2_MORE 0x80



#endif // LEDGER_APP_IOST_GLOBALS_H
