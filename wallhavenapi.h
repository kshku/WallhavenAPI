#ifndef WALLHAVEN_API_H
#define WALLHAVEN_API_H

#include <curl/curl.h>

typedef enum
{
    WALLHAVEN_OK,
    WALLHAVEN_CURL_FAIL,
} WallhavenCode;

typedef struct WallhavenAPI WallhavenAPI;

WallhavenAPI *wallhaven_init();
void wallhaven_free(WallhavenAPI *wa);

WallhavenCode wallhaven_apikey(WallhavenAPI *wa, const char *apikey);

#endif
