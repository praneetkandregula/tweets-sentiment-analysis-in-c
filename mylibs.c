#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>
#include <string.h>
#include <ctype.h>
#include "mylibs.h"

size_t buf_size = 20000;

int need_percent_encoding(char c)
{
    return !(isalnum((int)c) || c == '-' || c == '.' || c == '_' || c == '~');
}


char *percent_encode(char *str)
{
    for (unsigned int i = 0; str[i] != '\0'; i++) {
        if (need_percent_encoding(str[i])) {
            
            memmove(str + i + 3, str + i + 1, strlen(str) - i + 1);
            
            char tmp[2];
            snprintf(tmp, 2, "%X", str[i] >> 4);
            str[i + 1] = tmp[0];
            snprintf(tmp, 2, "%X", str[i] & 0xf);
            str[i + 2] = tmp[0];
            str[i] = '%';
        }
    }

    return str;
}

char *base64_encode(const char *msg, size_t msg_len)
{
    /* This could be improved. Currently it allocates bit too much. Times three
     * is required for the percent encoding but times four is just overestimate
     * of base64 encoding. */
    size_t signature_len = msg_len * 4 * 3 + 1;

    char *encoded = malloc(signature_len);
    if (encoded == NULL) {
        return NULL;
    }

    /* From https://www.openssl.org/docs/crypto/BIO_f_base64.html */
    BIO *b64 = BIO_new(BIO_f_base64());
    if (b64 == NULL) {
        free(encoded);
        return NULL;
    }

    /* Bio base64 writes to a stream for some reason, so open a stream to the
     * buffer. */
    FILE *stream = fmemopen(encoded, signature_len, "w");
    if (stream == NULL) {
        free(encoded);
        BIO_free_all(b64);
        return NULL;
    }

        BIO *bio = BIO_new_fp(stream, BIO_NOCLOSE);
    BIO_push(b64, bio);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, msg, (int)msg_len);
    BIO_flush(b64);

    BIO_free_all(b64);
    fclose(stream);

    return encoded;
}

char *generate_nonce(void)
{
    char nonce[32];

    if (!RAND_bytes((unsigned char *)nonce, sizeof(nonce))) {
        return NULL;
    }

    char *encoded_nonce = base64_encode(nonce, sizeof(nonce));
    if (encoded_nonce == NULL) {
        return NULL;
    }

    size_t nonce_len = strlen(encoded_nonce);
    for (size_t i = 0; i < nonce_len; i++) {
        if (!isalnum((int)encoded_nonce[i])) {
            /* Replace non-alphanumerics by arbitrary 'a' since they should not
             * be present in the nonce. */
            encoded_nonce[i] = 'a';
        }
    }

    return encoded_nonce;
}

unsigned char *hmac_sha1_encode(const char *data, const char *hmac_key,
                                       unsigned int *result_len)
{
    *result_len = 20; /* Should be always 20 bytes. */
    unsigned char *result = malloc(*result_len);
    if (result == NULL) {
        return NULL;
    }

    HMAC_CTX *ctx;

    /* Example from http://stackoverflow.com/a/245335. */
    ctx = HMAC_CTX_new();
    HMAC_CTX_reset(ctx);
    HMAC_Init_ex(ctx, hmac_key, (int)strlen(hmac_key), EVP_sha1(), NULL);
    HMAC_Update(ctx, (const unsigned char *)data, strlen(data));
    HMAC_Final(ctx, result, result_len);
    HMAC_CTX_free(ctx);

    return result;
}

/* Compute the signature from given parameters as required by the OAuth. */
char *compute_signature(const char *timestamp, const char *nonce, //const char *status,
                               const char *consumer_key, const char *auth_token,
                               const char *hmac_key)
{
    char *signature_base = malloc(buf_size);
    if (signature_base == NULL) {
        return NULL;
    }

    /* Encode the status again. */
    /*char encoded_status[strlen(status) * 3 + 1];
    snprintf(encoded_status, sizeof(encoded_status), "%s", status);
    percent_encode(encoded_status);*/
    
    //char url[128] = "/1.1/statuses/sample.json --header";
    //char percent_url[256];
    //strcpy(percent_url, percent_encode(url));
    int ret = snprintf(signature_base, buf_size, "GET&https%%3A%%2F%%2Fstream.twitter.com"
                       "%%2F1.1%%2Fstatuses%%2Fsample.json&"
                       "oauth_consumer_key%%3D%s%%26oauth_nonce%%3D%s"
                       "%%26oauth_signature_method%%3DHMAC-SHA1"
                       "%%26oauth_timestamp%%3D%s%%26oauth_token%%3D%s"
                       "%%26oauth_version%%3D1.0",
                       //"%%26status%%3D%s",
                       consumer_key, nonce, timestamp, auth_token); //encoded_status);
    if (ret < 0 || (size_t)ret > buf_size) {
        free(signature_base);
        return NULL;
    }

        unsigned int encoded_len;
    char *hmac_encoded = (char *)hmac_sha1_encode(signature_base, hmac_key,
                                                  &encoded_len);
    free(signature_base);

    if (hmac_encoded == NULL) {
        return NULL;
    }

    char *encoded = base64_encode(hmac_encoded, encoded_len);
    free(hmac_encoded);

    if (encoded == NULL) {
        return NULL;
    }

    return percent_encode(encoded);
}

char *create_post(const char *timestamp, const char *nonce, //const char *status,
                         const char *signature, const char *consumer_key,
                         const char *auth_token)
{
    char *post = malloc(buf_size);
    if (post == NULL) {
        return NULL;
    }

    int ret = snprintf(post, buf_size, //"POST /1.1/statuses/update.json HTTP/1.1\r\n"
                       //"User-Agent: LightBot\r\n"
                       //"Host: api.twitter.com\r\n"
                       //"Content-Type: application/x-www-form-urlencoded\r\n"
                       "authorization: OAuth oauth_consumer_key=\"%s\", oauth_nonce=\"%s\", "
                       "oauth_signature=\"%s\", oauth_signature_method=\"HMAC-SHA1\", "
                       "oauth_timestamp=\"%s\", oauth_token=\"%s\", oauth_version=\"1.0\"\r\n",
                       //"Content-Length: %zu\r\n\r\n"
                       //"status=%s",
                       consumer_key, nonce, signature, timestamp, auth_token);
                       //strlen("status=") + strlen(status), status);
    if (ret < 0 || (size_t)ret >= buf_size) {
        free(post);
        return NULL;
    }

    return post;
}

char *strnstr(const char *haystack, const char *needle, size_t len)
{
        int i;
        size_t needle_len;

        if (0 == (needle_len = strnlen(needle, len)))
                return (char *)haystack;

        for (i=0; i<=(int)(len-needle_len); i++)
        {
                if ((haystack[0] == needle[0]) &&
                        (0 == strncmp(haystack, needle, needle_len)))
                        return (char *)haystack;

                haystack++;
        }
        return NULL;
}
