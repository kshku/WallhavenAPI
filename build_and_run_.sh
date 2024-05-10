#!/bin/sh

# Build file I am using to test

gcc -D DEBUG -D APIKEY="\"Your API KEY\"" my_test.c wallhavenapi.c -o test -lcurl

[ $? -eq 0 ] && ./test
