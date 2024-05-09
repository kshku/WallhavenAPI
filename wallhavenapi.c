#include "wallhavenapi.h"

#include <stdlib.h>
#include <string.h>

#define checkp_return(x, r) \
    if (!(x))               \
    return r
#define curl_check_return(x, r) \
    if (x != CURLUE_OK)         \
    return r

struct WallhavenAPI
{
    CURL *curl;
    CURLU *url;
};

WallhavenAPI *wallhaven_init()
{
    WallhavenAPI *wa;
    checkp_return(wa = (WallhavenAPI *)malloc(sizeof(WallhavenAPI)), NULL);
    checkp_return(wa->curl = curl_easy_init(), NULL);
    checkp_return(wa->curl = curl_easy_init(), NULL);
    checkp_return(wa->url = curl_url(), NULL);
    curl_check_return(curl_url_set(wa->url, CURLUPART_URL, "https://wallhavenapi.cc/api/v1", CURLU_URLENCODE), NULL);
    curl_check_return(curl_easy_setopt(wa->curl, CURLOPT_CURLU, wa->url), NULL);
    return wa;
}

void wallhaven_free(WallhavenAPI *wa)
{
    curl_easy_cleanup(wa->curl);
    curl_url_cleanup(wa->url);
    free(wa);
}

WallhavenCode wallhaven_apikey(WallhavenAPI *wa, const char *apikey)
{
    size_t size = strlen(apikey) + strlen("apikey=") + 1;
    char *query = (char *)malloc(size);

    snprintf(query, size, "apikey=%s", apikey);
    curl_check_return(curl_url_set(wa->url, CURLUPART_QUERY, query, CURLU_APPENDQUERY), WALLHAVEN_CURL_FAIL);

    free(query);
    return WALLHAVEN_OK;
}
