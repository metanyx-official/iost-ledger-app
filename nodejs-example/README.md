# Smart card application protocol data unit
//"url": "https://test.api.iost.io" http://localhost:30001

In the context of smart cards, an application protocol data unit (APDU) is the communication unit between a smart card reader and a smart card. The structure of the APDU is defined by ISO/IEC 7816-4 Organization, security and commands for interchange.


## APDU message command-response pair

There are two categories of APDUs: command APDUs and response APDUs. A command APDU is sent by the reader to the card – it contains a mandatory 4-byte header (CLA, INS, P1, P2) and from 0 to 65 535 bytes of data. A response APDU is sent by the card to the reader – it contains from 0 to 65 536 bytes of data, and 2 mandatory status bytes (SW1, SW2).

| *Field name*  | *Length (bytes)* | *Description* |
| ------------- | -------------- | -----------
| CLA | 1 | Instruction class - indicates the type of command, e.g. interindustry or proprietary |
| INS | 1 | Instruction code - indicates the specific command, e.g. "write data" |
| P1-P2 | 2 | Instruction parameters for the command, e.g. offset into file at which to write the data |
| Lc | 0, 1 or 3 | Encodes the number (Nc) of bytes of command data to follow: 0 bytes denotes Nc=0, 1 byte with a value from 1 to 255 denotes Nc with the same value, 3 bytes, the first of which must be 0, denotes Nc in the range 1 to 65 535 (all three bytes may not be zero) |
| Command data | Nc | Nc bytes of data |
| Le | 0, 1, 2 or 3 | Encodes the maximum number (Ne) of response bytes expected: 0 bytes denotes Ne=0, 1 byte in the range 1 to 255 denotes that value of Ne, or 0 denotes Ne=256, 2 bytes (if extended Lc was present in the command) in the range 1 to 65535 denotes Ne of that value, or two zero bytes denotes 65536, 3 bytes (if Lc was not present in the command), the first of which must be 0, denote Ne in the same way as two-byte Le |

