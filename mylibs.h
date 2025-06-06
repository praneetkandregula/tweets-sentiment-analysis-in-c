#include <ctype.h>
int need_percent_encoding(char c);
char *percent_encode(char *str);
char *base64_encode(const char *msg, size_t msg_len);
char *generate_nonce(void);
unsigned char *hmac_sha1_encode(const char *data, const char *hmac_key, unsigned int *result_len);
char *compute_signature(const char *timestamp, const char *nonce, const char *consumer_key, const char *auth_token, const char *hmac_key);
char *create_post(const char *timestamp, const char *nonce, const char *signature, const char *consumer_key, const char *auth_token);
char *strnstr(const char *haystack, const char *needle, size_t len);