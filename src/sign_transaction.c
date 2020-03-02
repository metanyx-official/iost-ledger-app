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

static struct
{
    // ui common
//    uint32_t key_index;
    int bip_32_length;
    uint32_t bip_32_path[BIP32_PATH_LENGTH];

    // temp variables
    uint8_t transfer_to_index;

    // ui_transfer_tx_approve
    char ui_tx_approve_l1[40];
    char ui_tx_approve_l2[40];
    char ui_tx_approve_l3[40];
    char ui_tx_approve_l4[40];

    // what step of the UI flow are we on
    bool do_sign;

    // verify account transaction
    bool do_verify;

    // Raw transaction from APDU
    uint8_t raw_transaction[MAX_TX_SIZE];
    uint16_t raw_transaction_length;
} context;

//#if defined(TARGET_NANOS)

//unsigned int ui_tx_approve_button(
//    unsigned int button_mask,
//    unsigned int button_mask_counter
//);

//#elif defined(TARGET_NANOX)

//unsigned int io_seproxyhal_confirm_tx_approve(const bagl_element_t *e);
//unsigned int io_seproxyhal_confirm_tx_reject(const bagl_element_t *e);

//#endif // TARGET

//void handle_transaction_body();

#if defined(TARGET_NANOS)
// UI definition for Nano S
static const bagl_element_t ui_tx_approve[] = {
    UI_BACKGROUND(),
    UI_ICON_LEFT(0x00, BAGL_GLYPH_ICON_CROSS),
    UI_ICON_RIGHT(0x00, BAGL_GLYPH_ICON_CHECK),

    // X                  O
    //   Line 1
    //   Line 2

    UI_TEXT(0x00, 0, 12, 128, context.ui_tx_approve_l1),
    UI_TEXT(0x00, 0, 26, 128, context.ui_tx_approve_l2)
};

// Each UI element has a macro defined function that is its
// button handler, which must be named after the element with _button
// appended. This function is called on every single iteration of 
// the app loop while the async reply flag is set. The events consume
// that flag and allow the app to continue. 
unsigned int ui_tx_approve_button(
    unsigned int button_mask,
    unsigned int button_mask_counter
) {
    switch (button_mask) {
        case BUTTON_EVT_RELEASED | BUTTON_LEFT:
            io_exchange_status(SW_USER_REJECTED, 0);
            ui_idle();

            break;

        case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
            if (context.do_sign) {
                // Step 2
                iost_sign(
                    context.bip_32_path,
                    context.bip_32_length,
                    context.raw_transaction,
                    context.raw_transaction_length,
                    G_io_apdu_buffer
                );
            
                io_exchange_status(SW_OK, 64);
                ui_idle();
            } else {
                // Step 1
                // Signify "do sign" and change UI text
                context.do_sign = true;

                // if this is a verify account transaction (1 Sender, 0 Value)
                // then format for account verification
                if (context.do_verify) {
                    iost_snprintf(context.ui_tx_approve_l1, 40, "Verify Account ID");
                } else {
                    // Format for Signing a Transaction
                    iost_snprintf(context.ui_tx_approve_l1, 40, "Sign Transaction");
                }

                iost_snprintf(context.ui_tx_approve_l2, 40, "with Key #%u?", context.bip_32_path[context.bip_32_length - 1]);

                UX_REDISPLAY();
            }

            break;
    }

    return 0;
}

#elif defined(TARGET_NANOX)

unsigned int io_seproxyhal_confirm_tx_approve(const bagl_element_t *e) {
    hedera_sign(
        context.key_index,
        context.raw_transaction,
        context.raw_transaction_length,
        G_io_apdu_buffer
    );
    io_exchange_with_code(EXCEPTION_OK, 64);
    ui_idle();
    return 0;
}

unsigned int io_seproxyhal_confirm_tx_reject(const bagl_element_t *e) {
     io_exchange_with_code(EXCEPTION_USER_REJECTED, 0);
     ui_idle();
     return 0;
}

UX_STEP_NOCB(
    ux_confirm_tx_flow_1_step,
    bnn,
    {
        "Transaction Details",
        context.ui_tx_approve_l1,
        context.ui_tx_approve_l2
    }
);

UX_STEP_NOCB(
    ux_confirm_tx_flow_2_step,
    bnn,
    {
        "Confirm Transaction",
        context.ui_tx_approve_l3,
        context.ui_tx_approve_l4
    }
);


UX_STEP_VALID(
    ux_confirm_tx_flow_3_step,
    pb,
    io_seproxyhal_confirm_tx_approve(NULL),
    {
        &C_icon_validate_14,
        "Accept"
    }
);

UX_STEP_VALID(
    ux_confirm_tx_flow_4_step,
    pb,
    io_seproxyhal_confirm_tx_reject(NULL),
    {
        &C_icon_crossmark,
        "Reject"
    }
);

UX_DEF(
    ux_confirm_tx_flow,
    &ux_confirm_tx_flow_1_step,
    &ux_confirm_tx_flow_2_step,
    &ux_confirm_tx_flow_3_step,
    &ux_confirm_tx_flow_4_step
);

#endif

void handle_transaction_body() {
#if defined(TARGET_NANOS)
    // init at sign step 1, not verifying
    context.do_sign = false;
    context.do_verify = false;
#elif defined(TARGET_NANOX)
    // init key line for nano x
    hedera_snprintf(context.ui_tx_approve_l3, 40, "Sign Transaction");
    hedera_snprintf(context.ui_tx_approve_l4, 40, "with Key #%u?", context.key_index);
#endif
    // Handle parsed protobuf message of transaction body

    switch (context.raw_transaction_length) {
        case 1:
        // It's a "Create Account" transaction
//        case HederaTransactionBody_cryptoCreateAccount_tag:
//            hedera_snprintf(context.ui_tx_approve_l1, 40, "Create Account");
//            hedera_snprintf(
//                    context.ui_tx_approve_l2, 40, "with %s hbar?",
//                    hedera_format_tinybar(context.transaction.data.cryptoCreateAccount.initialBalance));
            break;

        // It's a "Transfer" transaction
        case 2: {
//        case HederaTransactionBody_cryptoTransfer_tag: {
//            if (context.transaction.data.cryptoTransfer.transfers.accountAmounts_count > 2) {
//                // Unsupported (number of accounts > 2)
//                THROW(EXCEPTION_MALFORMED_APDU);
//            }

//            // It's actually a "Verify Account" transaction (login)
//            if ( // Only 1 Account (Sender), Fee 1 Tinybar, and Value 0 Tinybar
//                context.transaction.data.cryptoTransfer.transfers.accountAmounts[0].amount == 0 &&
//                context.transaction.data.cryptoTransfer.transfers.accountAmounts_count == 1 &&
//                context.transaction.transactionFee == 1) {

//                #if defined(TARGET_NANOS)
//                    context.do_verify = true;
//                #elif defined(TARGET_NANOX)
//                    hedera_snprintf(context.ui_tx_approve_l3, 40, "Verify Account ID");
//                #endif  

//                hedera_snprintf(
//                    context.ui_tx_approve_l1,
//                    40,
//                    "Confirm Account"
//                );

//                hedera_snprintf(
//                    context.ui_tx_approve_l2,
//                    40,
//                    "%llu.%llu.%llu?",
//                    context.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.shardNum,
//                    context.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.realmNum,
//                    context.transaction.data.cryptoTransfer.transfers.accountAmounts[0].accountID.accountNum
//                );
//            } else {
//                // It's a transfer transaction between two parties
//                // Find sender based on positive tx amount
//                context.transfer_to_index = 1;
//                if (context.transaction.data.cryptoTransfer.transfers.accountAmounts[0].amount > 0) {
//                    context.transfer_to_index = 0;
//                }

//                hedera_snprintf(
//                        context.ui_tx_approve_l1,
//                        40,
//                        "Transfer %s hbar",
//                        hedera_format_tinybar(
//                                context.transaction.data.cryptoTransfer.transfers.accountAmounts[context.transfer_to_index].amount)
//                );

//                hedera_snprintf(
//                        context.ui_tx_approve_l2,
//                        40,
//                        "to %llu.%llu.%llu?",
//                        context.transaction.data.cryptoTransfer.transfers.accountAmounts[context.transfer_to_index].accountID.shardNum,
//                        context.transaction.data.cryptoTransfer.transfers.accountAmounts[context.transfer_to_index].accountID.realmNum,
//                        context.transaction.data.cryptoTransfer.transfers.accountAmounts[context.transfer_to_index].accountID.accountNum
//                );
//            }
        } break;

        default:
            // Unsupported
            // TODO: Better exception num
            THROW(SW_CLA_NOT_SUPPORTED);
    }

#if defined(TARGET_NANOS)
    UX_DISPLAY(ui_tx_approve, NULL);
#elif defined(TARGET_NANOX)
    ux_flow_init(0, ux_confirm_tx_flow, NULL);
#endif
}

// Handle parsing APDU and displaying UI element
void handle_sign_transaction(
    const uint8_t p1,
    const uint8_t p2,
    const uint8_t* const buffer,
    const uint16_t buffer_length,
    volatile uint8_t* flags,
    volatile uint16_t* tx
) {
    PRINTF("1Signing...%u bytes\n", buffer_length);
    // Read BIP32 path
    context.bip_32_length = io_read_bip32(buffer, buffer_length, context.bip_32_path);
    PRINTF("BIP32. length. %u\n", context.bip_32_length);
    // Key Index
//    context.key_index = U4LE(buffer, 0);
    // Raw Tx Length
    context.raw_transaction_length = buffer_length - context.bip_32_length;
    PRINTF("Raw Tx Length. %u\n", context.raw_transaction_length);
    // Oops Oof Owie
    if (context.raw_transaction_length > MAX_TX_SIZE) {
        THROW(SW_WRONG_LENGTH);
    }

    // Extract Transaction Message
    os_memmove(
        context.raw_transaction,
        (buffer + context.bip_32_length),
        context.raw_transaction_length
    );
//    // Make in memory buffer into stream
//    pb_istream_t stream = pb_istream_from_buffer(
//        context.raw_transaction,
//        context.raw_transaction_length
//    );
//    // Decode the Transaction
//    if (!pb_decode(
//        &stream,
//        Transaction_fields,
//        &context.transaction
//    )) {
//        PRINTF("invalid pb bytes");
//        THROW(SW_DATA_INVALID);
//    }

    handle_transaction_body();

    if (p1 != P1_CONFIRM) {
        *tx += 1;
        THROW(SW_OK);
    }
    *flags |= IO_ASYNCH_REPLY;
}
void clear_context_sign_transaction()
{
    os_memset(&context, 0, sizeof(context));
}
