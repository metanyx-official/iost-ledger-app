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
    volatile uint16_t rx = 0;
    volatile uint16_t tx = 0;
    volatile uint8_t flags = 0;

    for (;;) {
        volatile uint16_t sw = 0;

        BEGIN_TRY {
            TRY {
                rx = tx;
                tx = 0; // ensure no race in catch_other if io_exchange throws an error
                PRINTF("Ex%u %u/%u bytes with flags %u\n", sw, rx, tx, flags);
                rx = io_exchange(CHANNEL_APDU | flags, rx);
                flags = 0;

                if (rx < APDU_MIN_SIZE) {
                    THROW(EXCEPTION_IO_RESET);
                } else if (G_io_apdu_buffer[OFFSET_CLA] != CLA) {
                    PRINTF("malformed APDU\n");
                    THROW(SW_CLA_NOT_SUPPORTED);
                } else {
                    // APDU handler functions defined in handlers
                    const uint8_t p1 = G_io_apdu_buffer[OFFSET_P1];
                    const uint8_t p2 = G_io_apdu_buffer[OFFSET_P2];
                    const uint8_t* buffer = G_io_apdu_buffer + OFFSET_CDATA;
                    const uint16_t length = G_io_apdu_buffer[OFFSET_LC];

                    PRINTF("New APDU request:\n%.*H\n", length, G_io_apdu_buffer);

                    if (length == 0) {
                        PRINTF("empty BIP32 path\n");
                        THROW(SW_WRONG_LENGTH);
                    }
                    if (length + APDU_MAX_SIZE != rx) {
                        PRINTF("invalid APDU size: %d != %d\n", length + APDU_MAX_SIZE, rx);
                        THROW(SW_WRONG_LENGTH);
                    }

                    switch (G_io_apdu_buffer[OFFSET_INS]) {
                    case INS_RESET:
                        flags |= IO_RESET_AFTER_REPLIED;
                        THROW(SW_OK);
                        break;
                    case INS_ECHO:
                        tx = rx;
                        THROW(SW_OK);
                        break;
                    case INS_GET_CONFIGURATION:
                        handle_get_configuration(p1, p2, buffer, length, &flags, &tx);
                        break;
                    case INS_GET_PUBLIC_KEY:
                        handle_get_public_key(p1, p2, buffer, length, &flags, &tx);
                        break;
                    case INS_SIGN_TRANSACTION:
                    case INS_SIGN_TRX_HASH:
                        handle_sign_transaction(p1, p2, buffer, length, &flags, &tx);
                        break;
                    case INS_RETURN_TO_DASHBOARD:
                        goto return_to_dashboard;
                    default:
                        THROW(SW_INS_NOT_SUPPORTED);
                        break;
                    }
                }
            }
            CATCH(EXCEPTION_IO_RESET) {
                PRINTF("resetting\n");
                THROW(EXCEPTION_IO_RESET);
            }
            CATCH_OTHER(e) {
                PRINTF("clearing on %u\n", e);
                clear_context_get_configuration();
                clear_context_get_public_key();
                clear_context_sign_transaction();
                // Convert exception to response code and add to APDU return
                switch (e & 0xF000) {
                    case 0x6000:
                    case 0x9000:
                        sw = e;
                        break;
                    default:
                        sw = 0x6800 | (e & 0x7FF);
                        break;
                }
                io_set_status(sw, NULL, &tx);
            }
            FINALLY {
                // explicitly do nothing
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
                // explicitly do nothing
            }
        }
        END_TRY;
    }

    app_exit();

    return 0;
}
