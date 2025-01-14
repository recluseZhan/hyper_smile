#include "Crypto.h"

void decryptMessage(char *encMessageIn, size_t len, char *decMessageOut, size_t lenOut)
{
    uint8_t *encMessage = (uint8_t *) encMessageIn;
    uint8_t p_dst[BUFLEN] = {0};

    sgx_rijndael128GCM_decrypt(
        &aes_key,
        encMessage + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE,
        lenOut,
        p_dst,
        encMessage + SGX_AESGCM_MAC_SIZE, SGX_AESGCM_IV_SIZE,
        NULL, 0,
        (sgx_aes_gcm_128bit_tag_t *) encMessage);
    memcpy(decMessageOut, p_dst, lenOut);
}

void encryptMessage(char *decMessageIn, size_t len, char *encMessageOut, size_t lenOut)
{
    uint8_t *origMessage = (uint8_t *) decMessageIn;
    uint8_t p_dst[BUFLEN] = {0};

    // Generate the IV (nonce)
    sgx_read_rand(p_dst + SGX_AESGCM_MAC_SIZE, SGX_AESGCM_IV_SIZE);

    sgx_rijndael128GCM_encrypt(
        &aes_key,
        origMessage, len,
        p_dst + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE,
        p_dst + SGX_AESGCM_MAC_SIZE, SGX_AESGCM_IV_SIZE,
        NULL, 0,
        (sgx_aes_gcm_128bit_tag_t *) (p_dst));
}

