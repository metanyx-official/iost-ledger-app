#include "iost.h"
#include "globals.h"
#include "io.h"
#include "printf.h"
#include <os.h>
#include <os_io_seproxyhal.h>
#include <cx.h>

#define HBAR 100000000
#define HASH_SHA3_SIZE ED25519_KEY_SIZE

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

    if (secret_key) {
        psk = secret_key;
    }
    if (public_key) {
        ppk = public_key;
    }
    //PRINTF("Get key private %p and public %p\n", secret_key, public_key);
    io_seproxyhal_io_heartbeat();

#ifdef TARGET_BLUE
    os_perso_derive_node_bip32(CX_CURVE_Ed25519, bip_32_path, bip_32_length, private_key, NULL);
#else
//    os_perso_derive_node_bip32_seed_key(HDW_ED25519_SLIP10, CX_CURVE_Ed25519, bip_32_path, bip_32_length, privateKeyData, NULL, (unsigned char*) "ed25519 seed", 12);
    os_perso_derive_node_bip32_seed_key(HDW_ED25519_SLIP10, CX_CURVE_Ed25519, bip_32_path, bip_32_length, private_key, NULL, NULL, 0);
#endif
    //PRINTF("Private key derived: %.*H\n", ED25519_KEY_SIZE, private_key);
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

uint16_t iost_sign(
    const uint32_t* const bip_32_path,
    const int bip_32_length,
    const uint8_t* const trx_hash,
    const uint8_t hash_length,
    uint8_t* signature
) {
    cx_ecfp_private_key_t sk = {};
    uint16_t signature_length = 0;

    // Get Keys
    if (hash_length != 0 && iost_derive_keypair(bip_32_path, bip_32_length, &sk, NULL) == 0) {
#if CX_APILEVEL >= 8
        signature_length = cx_eddsa_sign(&sk, CX_LAST, CX_SHA512, trx_hash, hash_length, NULL, 0, signature, HASH_SHA3_SIZE * 2, NULL);
#else
        signature_length = cx_eddsa_sign(&sk, NULL, CX_LAST, CX_SHA512, trx_hash, hash_length, signature);
#endif
        // Clear private key
        os_memset(&sk, 0, sizeof(sk));
    }

    return signature_length;
}

uint16_t iost_hash_bytes(
    const uint8_t * const in_bytes,
    const uint16_t in_length,
    uint8_t* hash_out
) {
    cx_sha3_t sha3 = {};
    uint16_t hash_length = 0;
    uint8_t hash_buffer[HASH_SHA3_SIZE] = {};

    if (
        cx_sha3_init(&sha3, 256) != 0 && //size in bits
        cx_hash(&sha3.header, 0, in_bytes, in_length, NULL, 0) == 0
    ) {
        hash_length = cx_hash(&sha3.header, CX_LAST, NULL, 0, hash_buffer, HASH_SHA3_SIZE);
        os_memmove(hash_out, hash_buffer, hash_length);
    }

    return hash_length;
}

void iost_extract_bytes_from_public_key(
    const cx_ecfp_public_key_t* public_key,
    uint8_t* bytes_out,
    uint16_t* key_length
) {
    for (int i = 0; i < ED25519_KEY_SIZE; i++) {
        bytes_out[i] = public_key->W[ED25519_KEY_SIZE * 2 - i];
    }

    if ((public_key->W[ED25519_KEY_SIZE] & 1) == 1) {
        bytes_out[ED25519_KEY_SIZE - 1] |= 0x80;
    }
    if (key_length != NULL) {
        *key_length = ED25519_KEY_SIZE;
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

