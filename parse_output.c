#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mylibs.h"
#include "analyse_sentiment.h"
#include "get_tweets.h"
#define wordlength 128
#define linelength 10000
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define BLU   "\x1B[34m"
#define RESET "\x1B[0m"

void extract_required(char *tweet_text, char *tweet);
void deleteChars(char *s, char c);
int deleteWord(char *str, char *rem);
int processTweet(char *tweet, int *n, char *result, char words[20][wordlength]);
int deleteWordn(char *s, char *word, int len);
int count_substr(char *str, char *find, int n);

int extract_analyse_tweets(int n, FILE *fptr)
{
    time_t start = clock();
    size_t len = 0;
    char *tmp = NULL;
    char tweet_texts[n][5000], tweet_usernames[n][256];
    char tweets[n][linelength], usernames[n][128];
    char *text_start = NULL, *text_end = NULL;
    char *username_start = NULL, *username_end = NULL;
    int i = 0;

    // Read output file line by line
    while (i < n && (getline(&tmp, &len, fptr)) != -1)
    {
        // Extract only tweet and username
        text_start = strstr(tmp, "\"text\"");
        text_end = strstr(tmp, ",\"source\"");
        username_start = strstr(tmp, "\"screen_name\":");
        username_end = strstr(tmp, ",\"location\"");
        if(text_start == NULL || text_end == NULL || username_start == NULL || username_end == NULL)
        {
            if(tmp) free(tmp);
            tmp = NULL;
            continue;
        }

        if (i < n && strncmp("{\"created_at\":", tmp, 14) == 0 && count_substr(text_start, "\\u", text_end - text_start) < 3)//strnstr(text_start, "\\u", text_end - text_start) == NULL)
        {
            strncpy(tweet_texts[i], text_start, text_end - text_start);
            strncpy(tweet_usernames[i], username_start, username_end - username_start);
            i++;
        }
        if(tmp) free(tmp);
        tmp = NULL;
    }

    // print if no tweet is extracted
    if (strcmp(tweet_texts[0], "") == 0)
    {
        printf("Could not find a tweet\n");
        return 0;
    }

    int size = 155287;
    FILE *sentiwordsfile = fopen("SentiWords_Sorted.txt", "r");
    char **sentiwords = malloc(size * sizeof(char*));
    if(sentiwords == NULL) { printf("malloc failed\n"); return -1; }
    for(int i = 0; i < size; ++i)
    {
        sentiwords[i] = malloc(wordlength * sizeof(char));
        if(sentiwords[i] == NULL) { printf("malloc failed\n"); return -1; }
    }
    double sentiment[size];
    int l = 0;
    len = 0;
    char *tmp2 = NULL, *saveptr = NULL;
    while((getline(&tmp2, &len, sentiwordsfile)) != -1)
    {
        strcpy(sentiwords[l], strtok_r(tmp2, "\t", &saveptr));
        sscanf(strtok_r(NULL, "\n", &saveptr), "%lf", &sentiment[l]);
        ++l;
        if(tmp2 != NULL) free(tmp2);
        tmp2 = NULL;
    }
    fclose(sentiwordsfile);

    int k = 0;
    int res2 = 0;
    int count = 0;
    double senti = 0;
    char *result;
    char words[20][wordlength];
    // Extract only tweet and process it
    for(int j=0; j < i; ++j)
    {
        k = 0;
        extract_required(tweet_texts[j], tweets[j]);
        extract_required(tweet_usernames[j], usernames[j]);
        result = malloc(strlen(tweets[j]) + 1);
        res2 = processTweet(tweets[j], &k, result, words);
        if(res2 != 0 || strcmp(words[0], "") == 0) continue;
        else
        {
            senti = sentiment_analyse(words, k, sentiwords, sentiment);
            if(1)// if(senti != 0.0)
            {
                printf("Username: @%s\n", usernames[j]);
                printf("Tweet: ");
                for(int b = 0; b < k; ++b)
                    printf("%s ", words[b]);
                printf("\n");
                if(senti > 0.0) printf("Sentiment: " BLU "Positive (%.3lf)\n\n" RESET, senti);
                else if(senti < 0.0) printf("Sentiment: " RED "Negative (%.3lf)\n\n" RESET, senti);
                else printf("Sentiment: " GRN "Neutral (%.3lf)\n\n" RESET, senti);
                ++count;
            }
        }
        free(result);
    }
    printf("%d tweets found.\n", count);
    for(int j = 0; j < size; ++j){ if(sentiwords[j]) free(sentiwords[j]); }
    if(sentiwords) free(sentiwords);
    time_t end = clock();
    double totaltime = (double) (end - start) / CLOCKS_PER_SEC;
    printf("Time taken: %lfs\n", totaltime);
    return 0;
}

int count_substr(char *str, char *find, int n)
{
    int count = 0, find_len = strlen(find);
    char *tmp = str;
    if(find_len == 0) return 0;
    while((tmp = strnstr(tmp, find, n)))
    {
        tmp += find_len;
        count++;
        // i+=find_len;
    }
    return count;
}

void extract_required(char *tweet_text, char *tweet)
{
    char delim[] = "\"";
	char *req = strtok(tweet_text, delim);
	req = strtok(NULL, delim); req = strtok(NULL, delim);
    strcpy(tweet, req);
}


void deleteChars(char *s, char c)
{
    int writer = 0, reader = 0;

    while (s[reader])
    {
        if (s[reader]!=c)
        {   
            s[writer++] = s[reader];
        }

        reader++;
    }

    s[writer]=0;
}

int deleteWord(char *str, char *rem)
{
    char *start = strstr(str, rem);
    if(start == NULL) return -1;
    char *end = strchr(start, ' ');
    if(end == NULL) end = strchr(start, '\0');
    
    int toRemoveLen = end - start;
    char *ptr = start;
    while(*(ptr+toRemoveLen+1))
    {
        *ptr = *(ptr+toRemoveLen+1);
        ptr++;
    }
    *ptr = 0;
    return 0;
}

int replace_word(char *str, char *word, char replace)
{
    int l;
    char *ptr = strstr(str, word);
    int index = 0;
    if(ptr != NULL) 
        index = str - ptr;
    else
        return -1;
    l = strlen(word);
    *ptr = replace;
    ptr++;
    while (*(ptr+l+1))
    {
        *ptr = *(ptr+l+1);
        ptr++;
    }
    *ptr = 0;
    return 0;
}

int processTweet(char *tweet, int *n, char *result, char words[20][wordlength])
{
    if(strlen(tweet) < 5) return -1;
    char punctuation[] = ".!,'?()/~_-:[]{}#";
    strcpy(result, tweet);
    
    // to lowercase
    char *tmp = result;
    for(; *tmp; ++tmp) *tmp = tolower(*tmp);

    while(1)
    {
        if(deleteWord(result, "http") == -1)
            break;
    }

    // while(1)
    // {
    //     if(deleteWord(result, "@") == -1)
    //         break;
    // }

    while(1)
    {
        int a = replace_word(result, "\\n", ' ');
        if(a == -1)
            break;
    }

    //remove punctuatuon symbols
    int pun_length = strlen(punctuation);
    for(int i = 0; i < pun_length; ++i)
    {
        deleteChars(result, punctuation[i]);
    }

    // separate words into a array of strings
    int j = 0, ctr = 0;
    for(int i=0;i<=(int)(strlen(result));i++)
    {
        if(result[i]==' '||result[i]=='\0')
        {
            words[ctr][j]='\0';
            ctr++;
            j=0;
        }
        else
        {
            words[ctr][j]=result[i];
            j++;
        }
    }

    *n = ctr;
    if(ctr <= 3)
    {
        return -1;
    }
    return 0;
}