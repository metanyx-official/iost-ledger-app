//#include "sign_transaction.h"
#include "handlers.h"
#include "globals.h"
#include "utils.h"
#include "printf.h"
#include "debug.h"
#include "errors.h"
#include "io.h"
#include "ui.h"
#include "iost.h"
#include <stdbool.h>

#if defined(TARGET_NANOS)
static struct sign_tx_context_t {
    uint8_t msg_body[MAX_MSG_SIZE];
    uint16_t msg_length;
    uint16_t signature_length;
    // Lines on the UI Screen
    // L1 Only used for title in Nano X compare
    char ui_approve_l2[DISPLAY_SIZE + 1];
} context;

static const bagl_element_t ui_tx_approve[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(LEFT_ICON_ID, BAGL_GLYPH_ICON_CROSS),
    UI_ICON_RIGHT(RIGHT_ICON_ID, BAGL_GLYPH_ICON_CHECK),
    //
    //    Sign Transaction
    //       Key #123?
    //
    UI_TEXT(LINE_1_ID, 0, 12, 128, "Sign Transaction"),
    UI_TEXT(LINE_2_ID, 0, 26, 128, context.ui_approve_l2),
};


static unsigned int ui_tx_approve_button(
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
        PRINTF("Write out %u bytes", context.signature_length + 1);
        os_memmove(G_io_apdu_buffer, context.msg_body, context.signature_length + 1);
        clear_context_sign_message();
        io_exchange_status(SW_OK, context.signature_length + 1);
        break;

    default:
        break;
    }

    return 0;
}

#elif defined(TARGET_NANOX)
unsigned int io_seproxyhal_touch_tx_ok(const bagl_element_t *e) {
    io_exchange_with_code(EXCEPTION_OK, 32);
    compare_pk();
    return 0;
}

unsigned int io_seproxyhal_touch_tx_cancel(const bagl_element_t *e) {
     io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
     ui_idle();
     return 0;
}

UX_STEP_NOCB(
    ux_approve_tx_flow_1_step,
    bn,
    {
        "Sign Transaction",
        context.ui_approve_l2
    }
);

UX_STEP_VALID(
    ux_approve_tx_flow_2_step,
    pb,
    io_seproxyhal_touch_pk_ok(NULL),
    {
       &C_icon_validate_14,
       "Approve"
    }
);

UX_STEP_VALID(
    ux_approve_tx_flow_3_step,
    pb,
    io_seproxyhal_touch_pk_cancel(NULL),
    {
        &C_icon_crossmark,
        "Reject"
    }
);

UX_STEP_CB(
    ux_compare_tx_flow_1_step,
    bnnn_paging,
    ui_idle(),
    {
        .title = "Sign With Key",
        .text = (char*) context.full_key
    }
);

UX_DEF(
    ux_approve_tx_flow,
    &ux_approve_tx_flow_1_step,
    &ux_approve_tx_flow_2_step,
    &ux_approve_tx_flow_3_step
);

UX_DEF(
    ux_compare_tx_flow,
    &ux_compare_tx_flow_1_step
);

void compare_pk() {
    ux_flow_init(0, ux_compare_tx_flow, NULL);
}

#endif // TARGET_NANOX

void read_message(
    const uint8_t* const buffer,
    const uint16_t buffer_length
) {
    if (context.msg_length + buffer_length > MAX_MSG_SIZE) {
        PRINTF("invalid message size %u + %u > %u\n", context.msg_length, buffer_length, MAX_MSG_SIZE);
        THROW(SW_WRONG_LENGTH);
    }

    PRINTF("Reading %u bytes\n", buffer_length);
    if (buffer_length != 0) {
        os_memmove(context.msg_body + context.msg_length, buffer, buffer_length);
        context.msg_length += buffer_length;
    }
}

void sign_message(
    const int hash_before_sign,
    uint8_t* signature
) {
    if (context.msg_length == 0) {
        PRINTF("empty message\n");
        THROW(SW_WRONG_LENGTH);
    }
    if (context.msg_length > MAX_MSG_SIZE) {
        PRINTF("invalid message size %u > %u\n", context.msg_length, MAX_MSG_SIZE);
        THROW(SW_WRONG_LENGTH);
    }

    // Read BIP32 path
    uint32_t bip_32_path[BIP32_PATH_LENGTH] = {};
    const uint16_t bip_32_length = io_read_bip32(context.msg_body, context.msg_length, bip_32_path);
    const uint16_t msg_offset = 1 + bip_32_length * sizeof(*bip_32_path);
    uint8_t* msg_body = context.msg_body + msg_offset;

    context.msg_length -= msg_offset;

    if (hash_before_sign != 0) {
        context.msg_length = iost_hash_bytes(msg_body, context.msg_length, msg_body);
    }

    context.signature_length = iost_sign(
        bip_32_path,
        bip_32_length,
        msg_body,
        context.msg_length,
        signature
    );
}

// Sign Handler
// Decodes and handles transaction message
void handle_sign_message(
    const uint8_t p1,
    const uint8_t p2,
    const uint8_t* const buffer,
    const uint16_t buffer_length,
    volatile uint8_t* flags,
    volatile uint16_t* tx
) {
    read_message(buffer, buffer_length);

    if ((p2 & P2_MORE) != P2_MORE || buffer_length == 0) {
        sign_message(buffer_length == 0, G_io_apdu_buffer);
    } else {
        // Wait for next call
        THROW(SW_OK);
    }

//    // Make in memory buffer into stream
//    pb_istream_t stream = pb_istream_from_buffer(
//        raw_transaction,
//        raw_transaction_length
//    );

//    // Decode the Transaction
//    if (!pb_decode(
//        &stream,
//        HederaTransactionBody_fields,
//        &ctx.transaction
//    )) {
//        // Oh no couldn't ...
//        THROW(EXCEPTION_MALFORMED_APDU);
//    }
//    handle_transaction_body();

    if (p1 != P1_CONFIRM) {
        *tx += context.signature_length + 1;
        PRINTF("Write out (%u): %.*H", *tx, *tx, G_io_apdu_buffer);
        clear_context_sign_message();
        THROW(SW_OK);
    } else if (context.signature_length + 1 > MAX_MSG_SIZE) {
        PRINTF("Signature length %u is greater then %u", context.signature_length + 1, MAX_MSG_SIZE);
        THROW(SW_WRONG_LENGTH);
    }

    // Read BIP32 path
    uint32_t bip_32_path[BIP32_PATH_LENGTH] = {};
    const uint16_t bip_32_length = io_read_bip32(context.msg_body, context.msg_length, bip_32_path);
    // Complete "Sign Transaction | With Key #x?"
    iost_snprintf(
        context.ui_approve_l2,
        DISPLAY_SIZE,
        "With Key #%u?",
        bip_32_length > 0
            ? bip_32_path[bip_32_length - 1] - BIP32_PATH_MASK
            : 0
    );

    PRINTF("Saving out (%u): %.*H", context.signature_length + 1, context.signature_length + 1, G_io_apdu_buffer);
    os_memmove(context.msg_body, G_io_apdu_buffer, context.signature_length + 1);


#if defined(TARGET_NANOS)
    UX_DISPLAY(ui_tx_approve, NULL);
#elif defined(TARGET_NANOX)
    ux_flow_init(0, ux_approve_tx_flow, NULL);
#endif // TARGET_NANOX

    *flags |= IO_ASYNCH_REPLY;
}
void clear_context_sign_message()
{
    os_memset(&context, 0, sizeof(context));
}
