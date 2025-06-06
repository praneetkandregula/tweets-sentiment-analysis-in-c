# Sentiment Analysis of Tweets in C

[![Build Status](https://travis-ci.com/Samyak2/sentiment-analysis-of-tweets-in-c.svg?branch=master)](https://travis-ci.com/Samyak2/sentiment-analysis-of-tweets-in-c)

This C program gets tweets, processes the output and performs basic sentiment analysis on it.

## Requirements

 - `OpenSSL` version 1.1.1c (with libssl and libcrypto)
 - `libcurl` version 7.64.0 (might be compatible with other versions)
 - `gcc` - tested on version 9.2.0
 - Linux or Unix based system - not tested on Windows

## Compiling

 - Run make to compile and generate executable file named `sentiment`.

    ```bash
    make
    ./sentiment
    ```

- To generate debug compatible executable, do

    ```bash
    export CC='gcc -g'
    make debug
    ```

## Usage

* Save your Twitter API keys in file named `keys` in this format:
    ```bash
    Consumer Key
    Consumer Secret
    Access Token
    Access Token Secret
    ```
* Download `SentiWords_1.1.txt` from [here](https://hlt-nlp.fbk.eu/technologies/sentiwords). Then sort it using 
    ```bash
     sort SentiWords_1.1.txt -o SentiWords_Sorted.txt
    ```
* Run `./client.out` and select your option
    1. **To get Tweets**: from the Twitter API and save the output to `output.json`.
    2. **To parse output and analyse sentiment**: Enter the maximum number of tweets required. This will parse the `output.json` file and will try to extract some tweets from it. It will then try to calculate the sentiment of a tweet by looking at it word by word. It then displays the tweets along with username and sentiment.
