#include "iost.h"
#include "globals.h"
#include "io.h"
#include "printf.h"
#include <os.h>
#include <os_io_seproxyhal.h>
#include <cx.h>

#define HBAR 100000000
#define HASH_SHA3_SIZE ED25519_KEY_SIZE

//void iost_transaction_add_action(struct _Transaction* tx, const char* contract, const char* abi, const void* data)
//{
//    Action act = new Action();
//       act.contract = contract;
//       act.action_name = abi;
//       JsonArray ja = new JsonArray();
//       Gson gson = new Gson();
//       act.data = gson.toJson(data);
//       this.actions.add(act);
//}


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

//    if (derive_private_key(&privateKey, bip32, bip32Len) == ED25519_KEY_SIZE) {
//        init_public_key(&privateKey, &publicKey, keyBuffer);
//        os_memset(&privateKey, 0, sizeof(cx_ecfp_private_key_t));
//    }
//}

uint16_t iost_derive_keypair(
    const uint32_t * const bip_32_path,
    const uint16_t bip_32_length,
    cx_ecfp_256_private_key_t* secret_key,
    cx_ecfp_256_public_key_t* public_key
) {
    uint16_t result = (uint16_t)(-1);
    cx_ecfp_public_key_t p_k = {};
    cx_ecfp_private_key_t s_k = {};
    cx_ecfp_public_key_t* ppk = &p_k;
    cx_ecfp_private_key_t* psk = &s_k;
    uint8_t private_key[ED25519_KEY_SIZE] = {};
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
    PRINTF("Private key derived: %.*H\n", ED25519_KEY_SIZE, private_key);
    io_seproxyhal_io_heartbeat();

    if (cx_ecfp_init_private_key(CX_CURVE_Ed25519, private_key, ED25519_KEY_SIZE, psk) > 0) {
        if (cx_ecfp_init_public_key(CX_CURVE_Ed25519, NULL, 0, ppk) == 0) {
            result = cx_ecfp_generate_pair(CX_CURVE_Ed25519, ppk, psk, 1);

            os_memset(private_key, 0, ED25519_KEY_SIZE);
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
    static uint8_t seed[ED25519_KEY_SIZE];
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

uint16_t iost_sign(
    const uint32_t* const bip_32_path,
    const int bip_32_length,
    const uint8_t* const trx_hash,
    const uint8_t hash_length,
//    const uint8_t* const trx_body,
//    const uint8_t trx_lengtn,
    uint8_t* result
) {
//    cx_sha3_t sha3 = {};
    cx_ecfp_private_key_t sk = {};
//    uint8_t trx_hash[HASH_SHA3_SIZE] = {};
    uint16_t signature_length = 0;
//    PRINTF("Hashing trx %.*H\n", trx_lengtn, trx_body);

//    if (cx_keccak_init(&sha3, 256) != 0) {
//        const uint16_t hash_length = cx_hash((cx_hash_t*)(&sha3), CX_LAST, trx_body, trx_lengtn, trx_hash, HASH_SHA3_SIZE);
        PRINTF("The trx hash is %.*H\n", hash_length, trx_hash);
        // Get Keys
        if (hash_length != 0 && iost_derive_keypair(bip_32_path, bip_32_length, &sk, NULL) == 0) {
PRINTF("----------->\n");
            // Sign Transaction
            // <cx.h> 2283
            // Claims to want Hashes, but other apps use the message itself
            // and complain that the documentation is wrong
#if CX_APILEVEL >= 8
            signature_length = cx_eddsa_sign(&sk, CX_LAST, CX_SHA512, trx_hash, hash_length, NULL, 0, result, HASH_SHA3_SIZE * 2, NULL);
#else
            signature_length = cx_eddsa_sign(&sk, NULL, CX_LAST, CX_SHA512, trx_hash, hash_length, result);
#endif

//            signature_length = (uint16_t)cx_eddsa_sign(
//                &sk,                // private key
//                0,                  // mode (UNSUPPORTED)
//                CX_SHA256,          // hashID
//                trx_hash,           // hash (really message)
//                hash_length,        // hash length (really message length)
//                NULL,               // context (UNUSED)
//                0,                  // context length (0)
//                result,             // signature
//                HASH_SHA3_SIZE * 2, // signature length
//                NULL                // info
//            );
PRINTF("!!!!!!!!!!\n");
            // Clear private key
            os_memset(&sk, 0, sizeof(sk));
        }
//    }
    PRINTF("Signature: %.*H\n", signature_length, result);
    return signature_length;
}

void public_key_to_bytes(
    const cx_ecfp_public_key_t* public_key,
    uint8_t* dst
) {
    for (int i = 0; i < ED25519_KEY_SIZE; i++) {
        dst[i] = public_key->W[ED25519_KEY_SIZE * 2 - i];
    }

    if (public_key->W[ED25519_KEY_SIZE] & 1) {
        dst[ED25519_KEY_SIZE - 1] |= 0x80;
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

