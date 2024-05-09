#include "wallhavenapi.h"

#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#define WALLPAPER_INFO_PATH "/api/v1/w/"
#define TAG_INFO_PATH "/api/v1/tag/"
#define USER_SETTINGS_PATH "/api/v1/settings"

// Check for Null pointer
#define checkp_return(x, r) \
    if (!(x))               \
    return r

// Check for error
// CURLUE_OK = CULRE_OK = WALLHAVEN_OK = 0
#define check_return(x, r) \
    if (x != 0)            \
    return r

struct WallhavenAPI
{
    CURL *curl;
    CURLU *url;
    const char *apikey;
};

typedef enum
{
    WALLPAPER,
    TAG,
    SETTINGS,
} PathTo;

// Helping functions

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
static CURLUcode append_query(WallhavenAPI *wa, const char *key, const char *value)
{
    size_t size = strlen(key) + 1 + strlen(value) + 1;
    char *query = (char *)malloc(size);

    snprintf(query, size, "%s=%s", key, value);
    CURLUcode r = curl_url_set(wa->url, CURLUPART_QUERY, query, CURLU_APPENDQUERY | CURLU_URLENCODE);
    free(query);

    return r;
}

// Get info of Wallpaper/Tag
static WallhavenCode get_info(WallhavenAPI *wa, const char *id, PathTo path_to)
{
    size_t size = strlen((path_to == WALLPAPER ? WALLPAPER_INFO_PATH : TAG_INFO_PATH)) + strlen(id) + 1;
    char *path = (char *)malloc(size);

    snprintf(path, size, (path_to == WALLPAPER ? WALLPAPER_INFO_PATH "%s" : TAG_INFO_PATH "%s"), id);

    check_return(curl_url_set(wa->url, CURLUPART_PATH, path, CURLU_URLENCODE), WALLHAVEN_CURL_FAIL);
    if (path_to == WALLPAPER && wa->apikey)
        check_return(append_query(wa, "apikey", wa->apikey), WALLHAVEN_CURL_FAIL);

    CURLcode c = curl_easy_perform(wa->curl);

#ifdef DEBUG
    curl_url_get(wa->url, CURLUPART_URL, &path, 0);
    printf("URL: %s\n", path);
    printf("CURLcode: %d\n", c);
#endif

    check_return(c, WALLHAVEN_CURL_FAIL);

    free(path);

    return WALLHAVEN_OK;
}

static WallhavenCode get_settings(WallhavenAPI *wa)
{
    check_return(curl_url_set(wa->url, CURLUPART_PATH, USER_SETTINGS_PATH, CURLU_URLENCODE), WALLHAVEN_CURL_FAIL);

    checkp_return(wa->apikey, WALLHAVEN_NO_API_KEY);

    check_return(append_query(wa, "apikey", wa->apikey), WALLHAVEN_CURL_FAIL);

    CURLcode c = curl_easy_perform(wa->curl);

#ifdef DEBUG
    char *url;
    curl_url_get(wa->url, CURLUPART_URL, &url, 0);
    printf("URL: %s\n", url);
    printf("CURLcode: %d\n", c);
    free(url);
#endif

    check_return(c, WALLHAVEN_CURL_FAIL);

    return WALLHAVEN_OK;
}

static CURLUcode reset(WallhavenAPI *wa)
{
    // Reset the queries and options
    curl_easy_reset(wa->curl);

    return curl_url_set(wa->url, CURLUPART_QUERY, NULL, 0);
}

static WallhavenCode curl_to_response(WallhavenAPI *wa, Response *response)
{
    // Write curl output to response
    check_return(curl_easy_setopt(wa->curl, CURLOPT_CURLU, wa->url), WALLHAVEN_CURL_FAIL);
    check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEFUNCTION, write_function), WALLHAVEN_CURL_FAIL);
    check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEDATA, (void *)response), WALLHAVEN_CURL_FAIL);

    return WALLHAVEN_OK;
}

static WallhavenCode curl_to_file(WallhavenAPI *wa, FILE *file)
{
    // Write curl ouput to a file
    check_return(curl_easy_setopt(wa->curl, CURLOPT_CURLU, wa->url), WALLHAVEN_CURL_FAIL);
    check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEFUNCTION, write_function_tofile), WALLHAVEN_CURL_FAIL);
    check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEDATA, (void *)file), WALLHAVEN_CURL_FAIL);

    return WALLHAVEN_OK;
}

// API implementation
WallhavenAPI *wallhaven_init()
{
    WallhavenAPI *wa;
    checkp_return(wa = (WallhavenAPI *)malloc(sizeof(WallhavenAPI)), NULL);

    checkp_return(wa->curl = curl_easy_init(), NULL);
    checkp_return(wa->url = curl_url(), NULL);
    check_return(curl_url_set(wa->url, CURLUPART_URL, "https://wallhaven.cc", CURLU_URLENCODE), NULL);

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
    check_return(reset(wa), WALLHAVEN_CURL_FAIL);

    check_return(curl_to_response(wa, response), WALLHAVEN_CURL_FAIL);

    return get_info(wa, id, WALLPAPER);
}

WallhavenCode wallhaven_wallpaper_info_tofile(WallhavenAPI *wa, const char *id, FILE *file)
{
    check_return(reset(wa), WALLHAVEN_CURL_FAIL);

    check_return(curl_to_file(wa, file), WALLHAVEN_CURL_FAIL);

    return get_info(wa, id, WALLPAPER);
}

WallhavenCode wallhaven_tag_info(WallhavenAPI *wa, const char *id, Response *response)
{
    check_return(reset(wa), WALLHAVEN_CURL_FAIL);

    check_return(curl_to_response(wa, response), WALLHAVEN_CURL_FAIL);

    return get_info(wa, id, TAG);
}

WallhavenCode wallhaven_tag_info_tofile(WallhavenAPI *wa, const char *id, FILE *file)
{
    check_return(reset(wa), WALLHAVEN_CURL_FAIL);

    check_return(curl_to_file(wa, file), WALLHAVEN_CURL_FAIL);

    return get_info(wa, id, TAG);
}

WallhavenCode wallhaven_get_user_settings(WallhavenAPI *wa, Response *response)
{
    check_return(reset(wa), WALLHAVEN_CURL_FAIL);

    check_return(curl_to_response(wa, response), WALLHAVEN_CURL_FAIL);

    return get_settings(wa);
}

WallhavenCode wallhaven_get_user_settings_tofile(WallhavenAPI *wa, FILE *file)
{
    check_return(reset(wa), WALLHAVEN_CURL_FAIL);

    check_return(curl_to_file(wa, file), WALLHAVEN_CURL_FAIL);

    return get_settings(wa);
}
