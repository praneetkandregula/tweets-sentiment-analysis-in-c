#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include "mylibs.h"
#include "get_tweets.h"
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);
int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

int count = 0;

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp)
{
    // Write to output file
    FILE *fptr;
    fptr = fopen("output.json", "a");
    fprintf(fptr,"%s", buffer);
    fclose(fptr);
    count++;
    return size * nmemb;
}

int progress_callback(void *clientp,   curl_off_t dltotal,   curl_off_t dlnow,   curl_off_t ultotal,   curl_off_t ulnow)
{
    if(count >= 200) return -1;
    return 0;
}

int tweets_get(long timeout)
{
    int buf_size = 20000;
    time_t t = time(NULL);
    char timestamp[11];
    snprintf(timestamp, sizeof(timestamp), "%zu", (size_t)t);

    FILE *fptr;
    fptr = fopen("output.json", "w");
    fprintf(fptr,"%s", "");
    fclose(fptr);

    char *nonce = malloc(buf_size);
    if (nonce == NULL) return 1;
    strcpy(nonce, generate_nonce());

    char *signature = malloc(buf_size);
    if (signature == NULL) return 1;

    // Get Twitter API keys from file 'keys'
    FILE *keys = fopen("keys", "r"); int len = 64;
    char consumer_key[len];
    char consumer_secret[len];
    char access_token[len];
    char access_token_secret[len];
    fgets(consumer_key, len, keys); consumer_key[strcspn(consumer_key, "\n")] = 0;
    fgets(consumer_secret, len, keys); consumer_secret[strcspn(consumer_secret, "\n")] = 0;
    fgets(access_token, len, keys); access_token[strcspn(access_token, "\n")] = 0;
    fgets(access_token_secret, len, keys); access_token_secret[strcspn(access_token_secret, "\n")] = 0;
    fclose(keys);

    // Calculate key from API keys
    char *hmac_key = malloc(buf_size);
    if(hmac_key == NULL) return 1;
    strcat(hmac_key, consumer_secret);
    strcat(hmac_key, "&");
    strcat(hmac_key, access_token_secret);

    // Calculate signature 
    strcpy(signature, compute_signature(timestamp, nonce, consumer_key, access_token, hmac_key));

    // Create request to be sent
    char *post = malloc(buf_size);
    if(post == NULL) return 1;
    strcpy(post, create_post(timestamp, nonce, signature, consumer_key, access_token));

    // Initialize curl
    CURL *curl;
    curl = curl_easy_init();
    if(curl) {
        // Set request paramters like url, header, timeout
        struct curl_slist *chunk = NULL;
        chunk = curl_slist_append(chunk, post);
        curl_easy_setopt(curl, CURLOPT_URL, "https://stream.twitter.com/1.1/statuses/sample.json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        // Send curl request
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    printf("Got %d lines\n", count);
    return 0;
}