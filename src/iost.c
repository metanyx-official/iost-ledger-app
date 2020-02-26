#include "iost.h"
#include "globals.h"
#include "printf.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "pb_common.h"
#include "IOST_api.pb.h"
#include <os.h>


void iost_transaction_add_action(struct _Transaction* tx, const char* contract, const char* abi, const void* data)
{
//    Action act = new Action();
//       act.contract = contract;
//       act.action_name = abi;
//       JsonArray ja = new JsonArray();
//       Gson gson = new Gson();
//       act.data = gson.toJson(data);
//       this.actions.add(act);
}

void derive_private_key(cx_ecfp_private_key_t *privateKey, uint32_t *bip32, uint8_t bip32Len)
{
    uint8_t privateKeyData[BIP32_SIZE];
    io_seproxyhal_io_heartbeat();
#ifdef TARGET_BLUE
    os_perso_derive_node_bip32(CX_CURVE_Ed25519, bip32, bip32Len, privateKeyData, NULL);
#else
    os_perso_derive_node_bip32_seed_key(HDW_ED25519_SLIP10, CX_CURVE_Ed25519, bip32, bip32Len, privateKeyData, NULL, (unsigned char*) "ed25519 seed", 12);
#endif
    io_seproxyhal_io_heartbeat();
    cx_ecfp_init_private_key(CX_CURVE_Ed25519, privateKeyData, BIP32_SIZE, privateKey);
    memset(privateKeyData, 0 , sizeof(privateKeyData));
}

void init_public_key(cx_ecfp_private_key_t *privateKey, cx_ecfp_public_key_t *publicKey, uint8_t *buffer)
{
    cx_ecfp_generate_pair(CX_CURVE_Ed25519, publicKey, privateKey, 1);

    // copy public key little endian to big endian
    uint8_t i;
    for (i = 0; i < BIP32_SIZE; i++) {
        buffer[i] = publicKey->W[64 - i];
    }
    if ((publicKey->W[BIP32_SIZE] & 1) != 0) {
        buffer[31] |= 0x80;
    }
}

void iost_derive_keypair(
    uint32_t index,
    /* out */ cx_ecfp_private_key_t* secret_key,
    /* out */ cx_ecfp_public_key_t* public_key)
{
    static uint8_t seed[BIP32_SIZE];
    static uint32_t path[BIP32_PATH];
    static cx_ecfp_private_key_t pk;

    path[0] = 44 | 0x80000000;
    path[1] = 3030 | 0x80000000;
    path[2] = index | 0x80000000;
    path[3] = 0x80000000;
    path[4] = 0x80000000;

    os_perso_derive_node_bip32_seed_key(
        HDW_ED25519_SLIP10, 
        CX_CURVE_Ed25519, 
        path, 
        5, 
        seed, 
        NULL, 
        NULL, 
        0
    );

    cx_ecfp_init_private_key(
        CX_CURVE_Ed25519, 
        seed, 
        sizeof(seed), 
        &pk
    );

    if (public_key) {
        cx_ecfp_init_public_key(
            CX_CURVE_Ed25519, 
            NULL, 
            0, 
            public_key
        );
        cx_ecfp_generate_pair(
            CX_CURVE_Ed25519, 
            public_key,
            &pk, 
            1
        );
    }

    if (secret_key) {
        *secret_key = pk;
    }

    os_memset(seed, 0, sizeof(seed));
    os_memset(&pk, 0, sizeof(pk));
}

void iost_sign(
    uint32_t index,
    const uint8_t* tx,
    uint8_t tx_len,
    /* out */ uint8_t* result) 
{
    static cx_ecfp_private_key_t pk;

    // Get Keys
    iost_derive_keypair(index, &pk, NULL);

    // Sign Transaction
    // <cx.h> 2283
    // Claims to want Hashes, but other apps use the message itself
    // and complain that the documentation is wrong
    cx_eddsa_sign(
        &pk,                             // private key
        0,                               // mode (UNSUPPORTED)
        CX_SHA512,                       // hashID
        tx,                              // hash (really message)
        tx_len,                          // hash length (really message length)
        NULL,                            // context (UNUSED)
        0,                               // context length (0)
        result,                          // signature
        64,                              // signature length
        NULL                             // info
    );

    // Clear private key
    os_memset(&pk, 0, sizeof(pk));
}

#define HBAR 100000000

char* iost_format_tinybar(uint64_t tinybar)
{
    #define HBAR_BUF_SIZE 15

    static char buf[HBAR_BUF_SIZE];
    static uint64_t hbar;
    static uint64_t hbar_f;
    static int cnt;

    hbar = (tinybar / HBAR);
    hbar_f = (tinybar % HBAR * 10000 / HBAR);

    cnt = iost_snprintf(buf, HBAR_BUF_SIZE, "%llu", hbar);

    if (hbar_f != 0) {
        cnt += iost_snprintf(buf + cnt, HBAR_BUF_SIZE - cnt, ".%.4llu", hbar_f);
    }

    buf[cnt] = 0;
    return buf;
}

