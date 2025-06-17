#include <stdio.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

int main()
{
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    /* 暗号化するためのデータ */
    unsigned char plaintext[] = "Hello, World!";
    int plaintext_len = strlen((char *)plaintext);

    /* 暗号化キーと初期化ベクトル */
    unsigned char key[16] = "0123456789abcdef";
    unsigned char iv[16] = "1234567890abcdef";

    /* 暗号化処理コンテキストの作成 */
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

    /* AES-128-CBC暗号化の初期化 */
    EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv);

    /* 暗号化結果を格納するためのバッファ */
    unsigned char ciphertext[128];
    int len;
    int ciphertext_len;

    /* 暗号化 */
    EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len);
    ciphertext_len = len;

    /* 暗号化の終了処理 */
    EVP_EncryptFinal_ex(ctx, ciphertext + len, &len);
    ciphertext_len += len;

    /* 暗号文を表示 */
    printf("Ciphertext: ");
    for (int i = 0; i < ciphertext_len; i++) {
        printf("%02x", ciphertext[i]);
    }
    printf("\n");

    /* 復号化処理コンテキストの作成 */
    EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv);

    /* 復号化結果を格納するためのバッファ */
    unsigned char decryptedtext[128];

    /* 復号化 */
    EVP_DecryptUpdate(ctx, decryptedtext, &len, ciphertext, ciphertext_len);
    int decrypted_len = len;

    /* 復号化の終了処理 */
    EVP_DecryptFinal_ex(ctx, decryptedtext + len, &len);
    decrypted_len += len;

    /* 復号文を表示 */
    decryptedtext[decrypted_len] = '\0';
    printf("Decrypted text: %s\n", decryptedtext);

    /* コンテキストの解放 */
    EVP_CIPHER_CTX_free(ctx);

    EVP_cleanup();
    ERR_free_strings();

    return 0;
}