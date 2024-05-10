#include "wallhavenapi.h"

#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#define WALLPAPER_INFO_PATH "/api/v1/w/"
#define TAG_INFO_PATH "/api/v1/tag/"
#define USER_SETTINGS_PATH "/api/v1/settings"
#define SEARCH_PATH "/api/v1/search"

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

static CURLUcode reset(WallhavenAPI *wa)
{
    // Reset the queries and options
    curl_easy_reset(wa->curl);
    return curl_url_set(wa->url, CURLUPART_QUERY, NULL, 0);
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

WallhavenCode wallhaven_write_to_response(WallhavenAPI *wa, Response *response)
{
    check_return(reset(wa), WALLHAVEN_CURL_FAIL);

    // Write curl output to response
    check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEFUNCTION, write_function), WALLHAVEN_CURL_FAIL);
    check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEDATA, (void *)response), WALLHAVEN_CURL_FAIL);

    return WALLHAVEN_OK;
}

WallhavenCode wallhaven_write_to_file(WallhavenAPI *wa, FILE *file)
{
    check_return(reset(wa), WALLHAVEN_CURL_FAIL);

    // Write curl ouput to a file
    check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEFUNCTION, write_function_tofile), WALLHAVEN_CURL_FAIL);
    check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEDATA, (void *)file), WALLHAVEN_CURL_FAIL);

    return WALLHAVEN_OK;
}

WallhavenCode wallhaven_get_result(WallhavenAPI *wa, Path p, const char *id)
{
    size_t size;
    char *path;

    switch (p)
    {
    case WALLPAPER_INFO:
        if (wa->apikey)
            check_return(append_query(wa, "apikey", wa->apikey), WALLHAVEN_CURL_FAIL);
    case TAG_INFO:
        size = 1 + strlen(id) + strlen(p == WALLPAPER_INFO ? WALLPAPER_INFO_PATH : TAG_INFO_PATH);
        path = (char *)malloc(size);
        snprintf(path, size, (p == WALLPAPER_INFO ? WALLPAPER_INFO_PATH "%s" : TAG_INFO_PATH "%s"), id);
        break;
    case SETTINGS:
        checkp_return(wa->apikey, WALLHAVEN_NO_API_KEY);
        check_return(append_query(wa, "apikey", wa->apikey), WALLHAVEN_CURL_FAIL);
        size = 1 + strlen(USER_SETTINGS_PATH);
        path = (char *)malloc(size);
        snprintf(path, size, USER_SETTINGS_PATH);
        break;
    }

    check_return(curl_url_set(wa->url, CURLUPART_PATH, path, CURLU_URLENCODE), WALLHAVEN_CURL_FAIL);
    check_return(curl_easy_setopt(wa->curl, CURLOPT_CURLU, wa->url), WALLHAVEN_CURL_FAIL);

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

WallhavenCode wallhaven_search(WallhavenAPI *wa, const Parameters *p)
{
#ifdef DEBUG
    printf(
        "Parameters:\n"
        "\tQuery:\n"
        "\t\tTags = %s\n"
        "\t\tExclude tags = %s\n"
        "\t\tInclude tags = %s\n"
        "\t\tUser name = %s\n"
        "\t\tId = %s\n"
        "\t\tType = %d\n"
        "\t\tlike = %s\n"
        "\tCategories = %d\n"
        "\tPurity = %d\n"
        "\tSorting = %d\n"
        "\tOrder = %d\n"
        "\tTopRange = %d\n"
        "\tColors = %s\n"
        "\tPage = %d\n"
        "\tSeed = %s\n",
        p->q->tags,
        p->q->exclude_tags,
        p->q->include_tags,
        p->q->user_name,
        p->q->id,
        p->q->type,
        p->q->like,
        p->categories,
        p->purity,
        p->sorting,
        p->order,
        p->toprange,
        p->colors,
        p->page,
        p->seed);
#endif
}
