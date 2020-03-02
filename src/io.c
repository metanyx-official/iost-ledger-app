#include "io.h"
#include "globals.h"
#include "handlers.h"
#include "errors.h"
#include "debug.h"
#include "utils.h"
#include <os.h>
#include <os_io_seproxyhal.h>

// Everything below this point is Ledger magic. And the magic isn't well-
// documented, so if you want to understand it, you'll need to read the
// source, which you can find in the sdk repo for your device.
// Fortunately, we are not meant to understand this.

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];


unsigned char io_event(unsigned char channel)
{
    UNUSED(channel);

    // Ledger docs recommend checking the canary on each io_event
    debug_check_stack_canary();

    // can't have more than one tag in the reply, not supported yet.
    switch (G_io_seproxyhal_spi_buffer[0]) {
        case SEPROXYHAL_TAG_FINGER_EVENT:
            UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
            break;

        case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT:
            UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
            break;

        case SEPROXYHAL_TAG_STATUS_EVENT:
            if (G_io_apdu_media == IO_APDU_MEDIA_USB_HID && !(U4BE(G_io_seproxyhal_spi_buffer, 3) & SEPROXYHAL_TAG_STATUS_EVENT_FLAG_USB_POWERED)) {
                THROW(EXCEPTION_IO_RESET);
            }

            UX_DEFAULT_EVENT();
            break;

        case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
            UX_DISPLAYED_EVENT({});
            break;

        case SEPROXYHAL_TAG_TICKER_EVENT:
            UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {});
            break;
            
        default:
            UX_DEFAULT_EVENT();
            break;
    }

    // close the event if not done previously (by a display or whatever)
    if (!io_seproxyhal_spi_is_status_sent()) {
        io_seproxyhal_general_status();
    }

    // command has been processed, DO NOT reset the current APDU transport
    return 1;
}

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
    switch (channel & ~(IO_FLAGS)) {
        case CHANNEL_KEYBOARD:
            break;

        // multiplexed io exchange over a SPI channel and TLV encapsulated protocol
        case CHANNEL_SPI:
            if (tx_len) {
                io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

                if (channel & IO_RESET_AFTER_REPLIED) {
                    reset();
                }
                return 0; // nothing received from the master so far (it's a tx transaction)
            } else {
                return io_seproxyhal_spi_recv(G_io_apdu_buffer,
                                            sizeof(G_io_apdu_buffer), 0);
            }

        default:
            THROW(INVALID_PARAMETER);
    }
    return 0;
}


uint16_t io_read_bip32(
    const uint8_t* buffer,
    const uint16_t buffer_length,
    uint32_t* bip_32_path
) {
    uint16_t bip_32_length = *buffer++;
    if (
        (bip_32_length < 0x01 || bip_32_length > APDU_MAX_SIZE) ||
        (sizeof(uint32_t) * bip_32_length > buffer_length - 1)
    ) {
        PRINTF("invalid BIP32 length: %u\n", bip_32_length);
        THROW(SW_WRONG_LENGTH);
    }
    for (unsigned int i = 0; i < bip_32_length; i++) {
        bip_32_path[i] = 0;
        bip_32_path[i] |= (uint32_t)(*buffer++ << 0x18);
        bip_32_path[i] |= (uint32_t)(*buffer++ << 0x10);
        bip_32_path[i] |= (uint32_t)(*buffer++ << 0x08);
        bip_32_path[i] |= (uint32_t)(*buffer++ << 0x00);
    }
    return bip_32_length;
 }

void io_set_status(
    const uint16_t sw,
    volatile uint8_t* flags,
    volatile uint16_t* tx
) {
    if (flags != NULL) {
        *flags |= IO_RETURN_AFTER_TX;
    }
    if (tx != NULL) {
        G_io_apdu_buffer[(*tx)++] = (uint8_t)(sw >> 8);
        G_io_apdu_buffer[(*tx)++] = (uint8_t)(sw & 0xFF);
    }
}

void io_exchange_status(
    const uint16_t sw,
    uint16_t tx
) {
    uint8_t flags = CHANNEL_APDU;
    io_set_status(sw, &flags, &tx);
    io_exchange(flags, tx);
}

