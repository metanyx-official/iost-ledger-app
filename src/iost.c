#include "iost.h"
#include "globals.h"
#include "io.h"
#include "printf.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "pb_common.h"
#include "IOST_api.pb.h"
#include <os.h>
#include <os_io_seproxyhal.h>
#include <cx.h>

#define HBAR 100000000

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


//static int init_public_key(cx_ecfp_private_key_t* private_key, cx_ecfp_public_key_t* public_key)
//{
//    int result = cx_ecfp_init_public_key(CX_CURVE_Ed25519, NULL, 0, public_key);
//    if (result > 0) {
//        result = cx_ecfp_generate_pair(CX_CURVE_Ed25519, public_key, private_key, 1);
//    } else {
//        result = -1;
//    }
//    return result;
    // copy public key little endian to big endian
//    uint8_t i;
//    for (i = 0; i < BIP32_KEY_LEN; i++) {
//        buffer[i] = publicKey->W[64 - i];
//    }
//    if ((publicKey->W[BIP32_KEY_LEN] & 1) != 0) {
//        buffer[31] |= 0x80;
//    }
//}

//void iost_get_keypair(uint8_t p1, uint8_t p2, uint8_t *dataBuffer, uint16_t dataLength, uint8_t *keyBuffer)
//{
//    cx_ecfp_public_key_t publicKey;
//    cx_ecfp_private_key_t privateKey;
//    uint32_t bip32[BIP32_MAX_LEN];
//    int bip32Len = io_read_bip32(dataBuffer, dataLength, bip32);

//    dataBuffer += 1 + bip32Len * 4;
//    dataLength -= 1 + bip32Len * 4;

//    if (derive_private_key(&privateKey, bip32, bip32Len) == BIP32_KEY_SIZE) {
//        init_public_key(&privateKey, &publicKey, keyBuffer);
//        os_memset(&privateKey, 0, sizeof(cx_ecfp_private_key_t));
//    }
//}

int iost_derive_keypair(
    const uint32_t * const bip_32_path,
    const int bip_32_length,
    cx_ecfp_256_private_key_t* secret_key,
    cx_ecfp_256_public_key_t* public_key
) {
    int result = -1;
    cx_ecfp_public_key_t p_k;
    cx_ecfp_private_key_t s_k;
    cx_ecfp_public_key_t* ppk = &p_k;
    cx_ecfp_private_key_t* psk = &s_k;
    uint8_t private_key[BIP32_KEY_SIZE];
    // uint32_t bip_32_path[BIP32_PATH_LENGTH] = {
    //     IOST_NET_TYPE | BIP32_PATH_MASK,
    //     IOST_COIN_ID | BIP32_PATH_MASK,
    //     key_index | BIP32_PATH_MASK,
    //     BIP32_PATH_MASK,
    //     BIP32_PATH_MASK
    // };

    if (secret_key) {
        psk = secret_key;
    }
    if (public_key) {
        ppk = public_key;
    }
    PRINTF("Get key private %p and public %p\n", secret_key, public_key);
    io_seproxyhal_io_heartbeat();

#ifdef TARGET_BLUE
    os_perso_derive_node_bip32(CX_CURVE_Ed25519, bip_32_path, bip_32_length, private_key, NULL);
#else
//    os_perso_derive_node_bip32_seed_key(HDW_ED25519_SLIP10, CX_CURVE_Ed25519, bip_32_path, bip_32_length, privateKeyData, NULL, (unsigned char*) "ed25519 seed", 12);
    os_perso_derive_node_bip32_seed_key(HDW_ED25519_SLIP10, CX_CURVE_Ed25519, bip_32_path, bip_32_length, private_key, NULL, NULL, 0);
#endif
    PRINTF("Private key derived: %.*H\n", BIP32_KEY_SIZE, private_key);
    io_seproxyhal_io_heartbeat();

    if (cx_ecfp_init_private_key(CX_CURVE_Ed25519, private_key, BIP32_KEY_SIZE, psk) > 0) {
        if (cx_ecfp_init_public_key(CX_CURVE_Ed25519, NULL, 0, ppk) == 0) {
            result = cx_ecfp_generate_pair(CX_CURVE_Ed25519, ppk, psk, 1);

            os_memset(private_key, 0, BIP32_KEY_SIZE);
            if (secret_key == NULL) {
                os_memset(&s_k, 0, sizeof(s_k));
            }
            if (public_key == NULL) {
                os_memset(&p_k, 0, sizeof(p_k));
            }
            io_seproxyhal_io_heartbeat();
        }
    }
    return result;
}

void iost_derive_keypair_old(
    uint32_t key_index,
    /* out */ cx_ecfp_private_key_t* secret_key,
    /* out */ cx_ecfp_public_key_t* public_key)
{
    static uint8_t seed[BIP32_KEY_SIZE];
    static uint32_t path[BIP32_PATH_LENGTH + 2];
    static cx_ecfp_private_key_t pk;

    path[0] = 44 | 0x80000000;
    path[1] = 291 | 0x80000000;
    path[2] = key_index | 0x80000000;
    path[3] = 0x0;
    path[4] = 0x0;

    os_perso_derive_node_bip32_seed_key(
        HDW_ED25519_SLIP10, 
        CX_CURVE_Ed25519, 
        path, 
        BIP32_PATH_LENGTH + 2,
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
    const uint32_t* const bip_32_path,
    const int bip_32_length,
    const uint8_t* trx,
    uint8_t trx_len,
    /* out */ uint8_t* result
) {
    cx_ecfp_private_key_t pk;

    // Get Keys
    if (iost_derive_keypair(bip_32_path, bip_32_length, &pk, NULL) == 0) {
        // Sign Transaction
        // <cx.h> 2283
        // Claims to want Hashes, but other apps use the message itself
        // and complain that the documentation is wrong
        cx_eddsa_sign(
            &pk,        // private key
            0,          // mode (UNSUPPORTED)
            CX_SHA512,  // hashID
            trx,         // hash (really message)
            trx_len,     // hash length (really message length)
            NULL,       // context (UNUSED)
            0,          // context length (0)
            result,     // signature
            64,         // signature length
            NULL        // info
        );
        // Clear private key
        os_memset(&pk, 0, sizeof(cx_ecfp_private_key_t));
    }
}

void public_key_to_bytes(
    const cx_ecfp_256_public_key_t* public_key,
    uint8_t* dst
) {
    for (int i = 0; i < 32; i++) {
        dst[i] = public_key->W[64 - i];
    }

    if (public_key->W[32] & 1) {
        dst[31] |= 0x80;
    }
}


//const char* iost_format_tinybar(const uint64_t tinybar)
//{
//    static char hbar_buf[15];
//    static int hbar_buf_size = (int) sizeof(hbar_buf);

//    int cnt;
//    uint64_t hbar;
//    uint64_t hbar_f;

//    hbar = (hbar_buf / HBAR);
//    hbar_f = (hbar_buf % HBAR * 10000 / HBAR);

//    cnt = iost_snprintf(hbar_buf, hbar_buf_size, "%llu", hbar);

//    if (hbar_f != 0) {
//        cnt += iost_snprintf(hbar_buf+ cnt, hbar_buf_size - cnt, ".%.4llu", hbar_f);
//    }

//    hbar_buf[cnt] = 0;
//    return hbar_buf;
//}

