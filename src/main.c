#include "globals.h"
#include "errors.h"
#include "handlers.h"
#include "ui.h"
#include "io.h"
#include "utils.h"
#include "debug.h"

// This is the main loop that reads and writes APDUs. It receives request
// APDUs from the computer, looks up the corresponding command handler, and
// calls it on the APDU payload. Then it loops around and calls io_exchange
// again. The handler may set the 'flags' and 'tx' variables, which affect the
// subsequent io_exchange call. The handler may also throw an exception, which
// will be caught, converted to an error code, appended to the response APDU,
// and sent in the next io_exchange call.

// Things are marked volatile throughout the app to prevent unintended compiler
// reording of instructions (since the try-catch system is a macro)

void app_main() {
    volatile unsigned int rx = 0;
    volatile unsigned int tx = 0;
    volatile unsigned int flags = 0;

    for (;;) {
        volatile unsigned short sw = 0;

        BEGIN_TRY {
            TRY {
                rx = tx;
                tx = 0; // ensure no race in catch_other if io_exchange throws an error
                rx = io_exchange(CHANNEL_APDU | flags, rx);
                flags = 0;

                if (rx < APDU_MIN_SIZE) {
                    PRINTF("no APDU received\n");
                    THROW(EXCEPTION_EMPTY_BUFFER);
                } else if (G_io_apdu_buffer[OFFSET_CLA] != CLA) {
                    PRINTF("malformed APDU\n");
                    THROW(EXCEPTION_CLA_NOT_SUPPORTED);
                } else {
                    // APDU handler functions defined in handlers
                    uint8_t p1 = G_io_apdu_buffer[OFFSET_P1];
                    uint8_t p2 = G_io_apdu_buffer[OFFSET_P2];
                    uint16_t len = G_io_apdu_buffer[OFFSET_LC];
                    uint8_t* buffer = G_io_apdu_buffer + OFFSET_CDATA;

                    if (len == 0) {
                        PRINTF("empty BIP32 path\n");
                        THROW(EXCEPTION_WRONG_LENGTH);
                    }
                    if (len + APDU_MAX_SIZE != rx) {
                        PRINTF("invalid APDU size: %d != %d\n", len + APDU_MAX_SIZE, rx);
                        THROW(EXCEPTION_WRONG_LENGTH);
                    }
                    if ((p1 != P1_ED25519) && (p1 != P1_SECP256K1)) {
                        PRINTF("%d != P1_ED25519 || %d != P1_SECP256K1\n", p1, p1);
                        THROW(EXCEPTION_INVALID_P1P2);
                    }
                    if ((p2 != P2_BASE58) && (p2 != P2_BINARY)) {
                        PRINTF("%d != P2_BASE58 || %d != P2_BINARY\n", p2, p2);
                        THROW(EXCEPTION_INVALID_P1P2);
                    }

                    PRINTF("New APDU request:\n%.*H\n", len, G_io_apdu_buffer);

                    switch (G_io_apdu_buffer[OFFSET_INS]) {
                    case INS_RESET:
                        flags |= IO_RESET_AFTER_REPLIED;
                        THROW(EXCEPTION_OK);
                        break;
                    case INS_ECHO:
                        tx = rx;
                        THROW(EXCEPTION_OK);
                        break;
                    case INS_GET_CONFIGURATION:
                        // handlers -> get_app_configuration
                        handle_get_configuration(p1, p2, buffer, len, &flags, &tx);
                        break;
                    case INS_GET_PUBLIC_KEY:
                        // handlers -> get_public_key
                        handle_get_public_key(p1, p2, buffer, len, &flags, &tx);
                        break;
                    case INS_SIGN_TRANSACTION:
                        // handlers -> sign_transaction
                        handle_sign_transaction(p1, p2, buffer, len, &flags, &tx);
                        break;
                    case INS_RETURN_TO_DASHBOARD:
                        goto return_to_dashboard;
                    default:
                        THROW(EXCEPTION_INS_NOT_SUPPORTED);
                        break;
                    }
                }
            }
            CATCH(EXCEPTION_IO_RESET) {
                THROW(EXCEPTION_IO_RESET);
            }
            CATCH_OTHER(e) {
                // Convert the exception to a response code. All error codes
                // start with 6, except for 0x9000, which is a special
                // "success" code. Every APDU payload should end with such a
                // code, even if no other data is sent.

                // If the first byte is not a 6, mask it with 0x6800 to
                // convert it to a proper error code.

                switch (e & 0xF000) {
                    case 0x6000:
                    case 0x9000:
                        sw = e;
                        break;

                    default:
                        sw = 0x6800 | (e & 0x7FF);
                        break;
                }
                tx = set_error_code(G_io_apdu_buffer, sw, tx);
            }
            FINALLY {
                // do nothing
            }
        }
        END_TRY;
    }

return_to_dashboard:
    return;
}

void app_exit(void) {
    // All os calls must be wrapped in a try catch context
    BEGIN_TRY_L(exit) {
        TRY_L(exit) {
            os_sched_exit(-1);
        }
        FINALLY_L(exit) {
            // explicitly do nothing
        }
    }
    END_TRY_L(exit);
}

__attribute__((section(".boot"))) int main() {
    // exit critical section (ledger magic)
    __asm volatile("cpsie i");

    // go with the overflow
    debug_init_stack_canary();

    os_boot();

    for (;;) {
        // Initialize the UX system
        UX_INIT();

        BEGIN_TRY {
            TRY {
                // Initialize the hardware abstraction layer (HAL) in 
                // the Ledger SDK
                io_seproxyhal_init();

                // Power Cycle (I think?)
                USB_power(0);
                USB_power(1);

                // Shows the main menu
                ui_idle();

                // Nano X (but not Blue, lol) has Bluetooth
#ifdef HAVE_BLE
                BLE_power(0, NULL);
                BLE_power(1, "Nano X");
#endif // HAVE_BLE

                // Actual Main Loop
                app_main();
            }
            CATCH(EXCEPTION_IO_RESET) {
                // reset IO and UX before continuing
                continue;
            }
            CATCH_ALL {
                break;
            }
            FINALLY {
                // do nothing
            }
        }
        END_TRY;
    }

    app_exit();

    return 0;
}
