#include "wallhavenapi.h"

#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

// Check for Null pointer
#define checkp_return(x, r) \
    if (!(x))               \
    return r

// Check the curl error
#define curl_check_return(x, r) \
    if (x != CURLUE_OK)         \
    return r

struct WallhavenAPI
{
    CURL *curl;
    CURLU *url;
    const char *apikey;
};

// Callback function to write data into Response struct
static size_t write_function(void *data, size_t size, size_t nmemb, void *clientp)
{
    size_t realsize = size * nmemb;
    Response *r = (Response *)clientp;

    char *ptr = realloc(r->value, r->size + realsize + 1);
    checkp_return(ptr, 0);

    r->value = ptr;
    memcpy(&(r->value[r->size]), data, realsize);
    r->size += realsize;
    r->value[r->size] = 0;

    return realsize;
}

// Callback function to write data into the file
static size_t write_function_tofile(void *data, size_t size, size_t nmemb, void *clientp)
{
    FILE *fp = clientp;
    return fwrite(data, size, nmemb, fp);
}

// Append query as key=value
static CURLcode append_query(WallhavenAPI *wa, const char *key, const char *value)
{
    size_t size = strlen(key) + 1 + strlen(value) + 1;
    char *query = (char *)malloc(size);

    snprintf(query, size, "%s=%s", key, value);
    CURLcode r = curl_url_set(wa->url, CURLUPART_QUERY, query, CURLU_APPENDQUERY | CURLU_URLENCODE);
    free(query);

    return r;
}

// Gets the wallpaper's info
static WallhavenCode wallpaper_info(WallhavenAPI *wa, const char *id)
{
    size_t size = strlen("/api/v1/w/") + strlen(id) + 1;
    char *path = (char *)malloc(size);

    snprintf(path, size, "/api/v1/w/%s", id);

    curl_check_return(curl_url_set(wa->url, CURLUPART_PATH, path, CURLU_URLENCODE), WALLHAVEN_CURL_FAIL);
    if (wa->apikey)
        curl_check_return(append_query(wa, "apikey", wa->apikey), WALLHAVEN_CURL_FAIL);

    CURLcode c = curl_easy_perform(wa->curl);
    printf("CURLcode = %d\n", c);
    curl_check_return(c, WALLHAVEN_CURL_FAIL);

#ifdef DEBUG
    curl_url_get(wa->url, CURLUPART_URL, &path, 0);
    printf("%s\n", path);
#endif

    free(path);

    return WALLHAVEN_OK;
}

WallhavenAPI *wallhaven_init()
{
    WallhavenAPI *wa;
    checkp_return(wa = (WallhavenAPI *)malloc(sizeof(WallhavenAPI)), NULL);
    checkp_return(wa->curl = curl_easy_init(), NULL);
    checkp_return(wa->url = curl_url(), NULL);
    curl_check_return(curl_url_set(wa->url, CURLUPART_URL, "https://wallhaven.cc", CURLU_URLENCODE), NULL);
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
    checkp_return(wa->apikey = apikey, WALLHAVEN_FAIL);
    return WALLHAVEN_OK;
}

WallhavenCode wallhaven_wallpaper_info(WallhavenAPI *wa, const char *id, Response *response)
{
    // Reset the queries and options
    curl_check_return(curl_url_set(wa->url, CURLUPART_QUERY, NULL, 0), WALLHAVEN_CURL_FAIL);
    curl_easy_reset(wa->curl);

    curl_check_return(curl_easy_setopt(wa->curl, CURLOPT_CURLU, wa->url), WALLHAVEN_CURL_FAIL);
    curl_check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEFUNCTION, write_function), WALLHAVEN_CURL_FAIL);
    curl_check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEDATA, (void *)response), WALLHAVEN_CURL_FAIL);
    return wallpaper_info(wa, id);
}

WallhavenCode wallhaven_wallpaper_info_tofile(WallhavenAPI *wa, const char *id, FILE *file)
{
    // Reset the queries
    curl_check_return(curl_url_set(wa->url, CURLUPART_QUERY, NULL, 0), WALLHAVEN_CURL_FAIL);
    curl_easy_reset(wa->curl);

    curl_check_return(curl_easy_setopt(wa->curl, CURLOPT_CURLU, wa->url), WALLHAVEN_CURL_FAIL);
    curl_check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEFUNCTION, write_function_tofile), WALLHAVEN_CURL_FAIL);
    curl_check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEDATA, (void *)file), WALLHAVEN_CURL_FAIL);
    return wallpaper_info(wa, id);
}
