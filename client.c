#include <stdio.h>
#include "get_tweets.h"
#include "parse_output.h"

int main()
{
    int get;
    long timeout = 10L;
    int n = 0;
    int readmore = 0;
    FILE *fp = fopen("output.json", "r");
    if(fp == NULL) return -1;
    do
    {
        printf("Select option:\n");
        printf("1. Get Tweets from Twitter API.\n");
        printf("2. Extract tweets and analyse sentiment.\n");
        printf("0. Exit\n");
        scanf("%d", &get);
        switch (get)
        {
            case 1:
                printf("Please wait...\n");
                tweets_get(timeout);
                fp = fopen("output.json", "r");
                break;
            case 2:
                printf("Enter number of lines to parse: ");
                scanf("%d", &n);
                extract_analyse_tweets(n, fp);
                break;
            default:
                break;
        }
    } while (get != 0);
    fclose(fp);
    return 0;
}