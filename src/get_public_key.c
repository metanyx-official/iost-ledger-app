#include "handlers.h"
#include "globals.h"
#include "handlers.h"
#include "utils.h"
#include "io.h"
#include "ui.h"
#include "errors.h"
#include "debug.h"
#include "printf.h"
#include "iost.h"
#include "base58.h"
#include <bagl.h>
#include <os.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
//#include "get_public_key.h"

static struct
{
    cx_ecfp_public_key_t public_key;
    uint16_t pk_length;
    // Lines on the UI Screen
    // L1 Only used for title in Nano X compare
    char ui_approve_l2[DISPLAY_SIZE + 1];
    // Public Key Compare
    uint8_t display_index;
    uint8_t partial_key[DISPLAY_SIZE + 1];
} context;

#if defined(TARGET_NANOS)

static const bagl_element_t ui_get_public_key_compare[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(LEFT_ICON_ID, BAGL_GLYPH_ICON_LEFT),
    UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_RIGHT),
    // <=                  =>
    //       Public Key
    //      <partial>
    //
    UI_TEXT(LINE_1_ID, 0, 12, 128, "Public Key"),
    UI_TEXT(LINE_2_ID, 0, 26, 128, context.partial_key)
};

static const bagl_element_t ui_get_public_key_approve[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(LEFT_ICON_ID, BAGL_GLYPH_ICON_CROSS),
    UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_CHECK),
    //
    //    Export Public
    //       Key #123?
    //
    UI_TEXT(LINE_1_ID, 0, 12, 128, "Export Public"),
    UI_TEXT(LINE_2_ID, 0, 26, 128, context.ui_approve_l2),
};


void shift_partial_key()
{
    os_memmove(
        context.partial_key,
        G_io_apdu_buffer + context.display_index,
        DISPLAY_SIZE
    );
}

static unsigned int ui_get_public_key_compare_button(
    unsigned int button_mask, 
    unsigned int button_mask_counter
) {
    UNUSED(button_mask_counter);
    switch (button_mask) {
        case BUTTON_LEFT: // Left
        case BUTTON_EVT_FAST | BUTTON_LEFT:
            if (context.display_index > 0) {
                context.display_index--;
            }
            shift_partial_key();
            UX_REDISPLAY();
            break;
        case BUTTON_RIGHT: // Right
        case BUTTON_EVT_FAST | BUTTON_RIGHT:
            if (context.display_index < context.pk_length - DISPLAY_SIZE) {
                context.display_index++;
            }
            shift_partial_key();
            UX_REDISPLAY();
            break;
        case BUTTON_EVT_RELEASED | BUTTON_LEFT | BUTTON_RIGHT: // Continue
            ui_idle();
            break;
    }
    return 0;
}

static const bagl_element_t* ui_prepro_get_public_key_compare(
    const bagl_element_t* element
) {
    if (
        (element->component.userid == LEFT_ICON_ID) &&
        (context.display_index == 0)
    ) {
        return NULL; // Hide Left Arrow at Left Edge
    }
    if (
        (element->component.userid == RIGHT_ICON_ID) &&
        (context.display_index == context.pk_length - DISPLAY_SIZE)
    ) {
        return NULL; // Hide Right Arrow at Right Edge
    }
    return element;
}

void compare_pk() {
    // init partial key str from full str
    os_memmove(context.partial_key, G_io_apdu_buffer, DISPLAY_SIZE);
    context.partial_key[DISPLAY_SIZE] = '\0';
    
    // init display index
    context.display_index = 0;

    // Display compare with button mask
    UX_DISPLAY(
        ui_get_public_key_compare, 
        ui_prepro_get_public_key_compare
    );
}

static unsigned int ui_get_public_key_approve_button(
    unsigned int button_mask, 
    unsigned int button_mask_counter
) {
    UNUSED(button_mask_counter);
    switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
        io_exchange_status(SW_USER_REJECTED, 0);
        ui_idle();
        break;

    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
        io_exchange_status(SW_OK, context.pk_length + 1);
        compare_pk();
        break;

    default:
        break;
    }

    return 0;
}

#elif defined(TARGET_NANOX)
unsigned int io_seproxyhal_touch_pk_ok(const bagl_element_t *e) {
    io_exchange_with_code(EXCEPTION_OK, 32);
    compare_pk();
    return 0;
}

unsigned int io_seproxyhal_touch_pk_cancel(const bagl_element_t *e) {
     io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
     ui_idle();
     return 0;
}

UX_STEP_NOCB(
    ux_approve_pk_flow_1_step,
    bn,
    {
        "Export Public",
        context.ui_approve_l2
    }
);

UX_STEP_VALID(
    ux_approve_pk_flow_2_step,
    pb,
    io_seproxyhal_touch_pk_ok(NULL),
    {
       &C_icon_validate_14,
       "Approve"
    }
);

UX_STEP_VALID(
    ux_approve_pk_flow_3_step,
    pb,
    io_seproxyhal_touch_pk_cancel(NULL),
    {
        &C_icon_crossmark,
        "Reject"
    }
);

UX_STEP_CB(
    ux_compare_pk_flow_1_step,
    bnnn_paging,
    ui_idle(),
    {
        .title = "Public Key",
        .text = (char*) context.full_key
    }
);

UX_DEF(
    ux_approve_pk_flow,
    &ux_approve_pk_flow_1_step,
    &ux_approve_pk_flow_2_step,
    &ux_approve_pk_flow_3_step
);

UX_DEF(
    ux_compare_pk_flow,
    &ux_compare_pk_flow_1_step
);

void compare_pk() {
    ux_flow_init(0, ux_compare_pk_flow, NULL);
}

#endif // TARGET_NANOX

void get_pk(
    const uint32_t* const bip_32_path,
    const uint16_t bip_32_length,
    const uint8_t p1,
    const uint8_t p2,
    uint8_t* output
) {
    if ((p1 != P1_CONFIRM) && (p1 != P1_SILENT)) {
        PRINTF("%d != P1_CONFIRM || %d != P1_SILENT\n", p1, p1);
        THROW(SW_INVALID_P1P2);
    }
    if ((p1 == P1_CONFIRM) && (p2 == P2_BIN)) {
        PRINTF("%d == P1_CONFIRM && %d == P2_BIN\n", p1, p2);
        THROW(SW_INVALID_P1P2);
    }

    // Derive key
    if (iost_derive_keypair(bip_32_path, bip_32_length, NULL, &context.public_key) != 0) {
        PRINTF("iost_derive_keypair failed\n");
        THROW(SW_INTERNAL_ERROR);
    }

    uint8_t pk[ED25519_KEY_SIZE] = {};
    iost_extract_bytes_from_public_key(&context.public_key, pk, &context.pk_length);
    PRINTF("Get PubKey(%u): %.*H\n", context.pk_length, context.pk_length, pk);

    // Put Key bytes in APDU buffer
    switch (p2) {
    case P2_HEX:
        context.pk_length = bin2hex(output, pk, context.pk_length);
        break;
    case P2_BASE58:
        context.pk_length = encode_base_58(pk, context.pk_length, output);
        break;
    default:
        os_memmove(output, pk, context.pk_length);
        break;
    }
    output[context.pk_length] = 0;
}

void handle_get_public_key(
    const uint8_t p1,
    const uint8_t p2,
    const uint8_t* const buffer,
    const uint16_t buffer_length,
    volatile uint8_t* flags,
    volatile uint16_t* tx
) {
    // Read BIP32 path
    uint32_t bip_32_path[BIP32_PATH_LENGTH];
    const uint16_t bip_32_length = io_read_bip32(buffer, buffer_length, bip_32_path);

    // Populate context with PK
    get_pk(bip_32_path, bip_32_length, p1, p2, G_io_apdu_buffer + *tx);

    if (
        (p1 != P1_CONFIRM) ||
        (p2 != P2_HEX && p2 != P2_BASE58)
    ) {
        *tx += context.pk_length + 1;
        clear_context_get_public_key();
        THROW(SW_OK);
    }
    // Complete "Export Public | Key #x?"
    iost_snprintf(
        context.ui_approve_l2,
        DISPLAY_SIZE,
        "Key #%u?",
        bip_32_length > 0
            ? bip_32_path[bip_32_length - 1] - BIP32_PATH_MASK
            : 0
    );
#if defined(TARGET_NANOS)
    UX_DISPLAY(ui_get_public_key_approve, NULL);
#elif defined(TARGET_NANOX)
    ux_flow_init(0, ux_approve_pk_flow, NULL);
#endif // TARGET_NANOS

    *flags |= IO_ASYNCH_REPLY;
}

void clear_context_get_public_key()
{
     os_memset(&context, 0, sizeof(context));
}

