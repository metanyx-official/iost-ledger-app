#include "handlers.h"
#include "io.h"
#include "errors.h"
#include <stdint.h>
#include <os.h>
#include <os_io_seproxyhal.h>

void handle_get_configuration(
    const uint8_t p1,
    const uint8_t p2,
    const uint8_t* const buffer,
    const uint16_t buffer_length,
    volatile uint8_t* flags,
    volatile uint16_t* tx
) {
    UNUSED(p1);
    UNUSED(p2);
    UNUSED(buffer);
    UNUSED(buffer_length);

    // storage allowed?
    G_io_apdu_buffer[(*tx)++] = 0;
    // version
    G_io_apdu_buffer[(*tx)++] = APPVERSION_M;
    G_io_apdu_buffer[(*tx)++] = APPVERSION_N;
    G_io_apdu_buffer[(*tx)++] = APPVERSION_P;

    clear_context_get_configuration();
    THROW(SW_OK);
//    io_set_status(SW_OK, flags, tx);
}

void clear_context_get_configuration()
{
    // No context
}
