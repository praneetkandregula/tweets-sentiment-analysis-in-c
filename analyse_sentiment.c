#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "analyse_sentiment.h"
#define datasize 155287

int binary_search(char **sentiwords, char *word)
{
    int low = 0, high = datasize-1, mid, res;
    while(low <= high)
    {
        mid = (low+high)/2;
        res = strcmp(word, sentiwords[mid]);
        if(res == 0) return mid;
        else if(res > 0)
        {
            low = mid + 1;
        }
        else if(res < 0)
        {
            high = mid - 1;
        }
    }
    return -1;
}

double sentiment_analyse(char words[20][wordlength], int n, char **sentiwords, double *sentiment)
{
    int i = 0;
    int bin;
    double total_sentiment = 0.0;
    while(i < n)
    {

        // Binary search
        bin = binary_search(sentiwords, words[i]);
        if(bin != -1)
        {
            total_sentiment += sentiment[bin];
        }
        ++i;
    }
    return total_sentiment;
}