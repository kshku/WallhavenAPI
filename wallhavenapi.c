#include "wallhavenapi.h"

#include <stdlib.h>
#include <string.h>

// NOTE: Haven't checked the portablility of this code.
#if defined(_WIN32) | defined(_WIN64)
#define WALLHAVEN_PLATFORM_WINDOWS
#elif defined(__APPLE__) | defined(__MACH__)
#define WALLHAVEN_PLATFORM_MACOS
#elif defined(__linux__)
#define WALLHAVEN_PLATFORM_LINUX
#else
#error "Platform is not supported"
#endif

#ifdef WALLHAVEN_PLATFORM_WINDOWS
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#elif defined(WALLHAVEN_PLATFORM_MACOS) | defined(WALLHAVEN_PLATFORM_LINUX)
#include <unistd.h>
#define sleep_ms(ms) usleep((ms) * 1000)
#endif

#include <curl/curl.h>

#define WALLPAPER_INFO_PATH "/api/v1/w/"
#define TAG_INFO_PATH "/api/v1/tag/"
#define USER_SETTINGS_PATH "/api/v1/settings"
#define SEARCH_PATH "/api/v1/search"
#define COLLECTIONS_PATH "/api/v1/collections"

// Check whetehr Null pointer and if yes return
#define checkp_return(x, r) \
    if (!(x))               \
    return r

// Check whether equals to zero and if yes return
// CURLUE_OK = CULRE_OK = WALLHAVEN_OK = 0
#define check_return(x, r) \
    if (x != 0)            \
    return r

struct WallhavenAPI
{
    CURL *curl;
    CURLU *url;
    const char *apikey;
    bool api_key_set;
    onMaxAPICallLimitError api_call_limit_error;
    Response *response;
    time_t start_time;
};

// Default function for handling api_max_call_limit_error
static bool default_api_call_limit(time_t *start_time)
{
    time_t t = time(NULL);
    struct tm *current_time = localtime(&t);
    int wait_time = 60 - difftime(t, *start_time) + 1;
    printf("Hit max api call limit, waiting for %d seconds before retrying...\n", wait_time);
    sleep_ms(wait_time * 1000);

    return true;
}

// Helping functions

// Callback function to write data into Response struct
static size_t
write_function(void *data, size_t size, size_t nmemb, void *clientp)
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
static WallhavenCode append_query(WallhavenAPI *wa, const char *key, const char *value)
{
    if (key == "apikey" && wa->api_key_set)
        return WALLHAVEN_OK;

    if (key == "apikey")
        wa->api_key_set = true;

    size_t size = strlen(key) + 1 + strlen(value) + 1;
    char *query = (char *)malloc(size);

    snprintf(query, size, "%s=%s", key, value);
    CURLUcode r = curl_url_set(wa->url, CURLUPART_QUERY, query, CURLU_APPENDQUERY | CURLU_URLENCODE);
    free(query);

    check_return(r, WALLHAVEN_CURL_FAIL);

    return WALLHAVEN_OK;
}

static WallhavenCode format_q(WallhavenAPI *wa, Query *q)
{
    size_t size = 0;
    size_t s;
    char *c = (char *)calloc(1, size);

    if (q->tags)
    {
        s = strlen(q->tags) + 2; // tags
        c = realloc(c, size + s);
        size += snprintf(c + size, s, "%s ", q->tags);
    }

    if (q->user_name)
    {
        s = strlen(q->user_name) + 3; // @Username
        c = realloc(c, size + s);
        size += snprintf(c + size, s, "@%s ", q->user_name);
    }

    if (q->type)
    {
        s = 10; // type:{jpg|png}
        c = realloc(c, size + s);
        size += snprintf(c + size, s, "type:%s ", (q->type == PNG ? "png" : "jpg"));
    }

    if (q->like)
    {
        s = strlen(q->like) + 7; // like:wallpaper id
        c = realloc(c, size + s);
        size += snprintf(c + size, s, "like:%s ", q->like);
    }

    if (q->id)
    {
        check_return(size, WALLHAVEN_USING_ID_IN_COMBINATION);
        s = strlen(q->id) + 5; // id:id
        c = realloc(c, size + s);
        size += snprintf(c + size, s, "id:%s ", q->id);
    }

    c[size > 0 ? --(size) : size] = 0;

#ifdef DEBUG
    printf("format_q\nSize=%d\nString='%s'\nstrlen=%d\n", size, c, strlen(c));
#endif
    WallhavenCode r = WALLHAVEN_OK;
    if (size > 0)
        r = append_query(wa, "q", c);

    free(c);

    return r;
}

static WallhavenCode format_categories(WallhavenAPI *wa, int categories)
{
    if (!categories)
        return WALLHAVEN_OK;
    char c[4] = {0};
    int ci = 0;
    for (int i = 2; i > -1; --i, categories >>= 1)
        c[i] = (categories & 1) ? '1' : '0';

#ifdef DEBUG
    printf("format_categories\n%s\n", c);
#endif

    check_return(append_query(wa, "categories", c), WALLHAVEN_CURL_FAIL);
    return WALLHAVEN_OK;
}

static WallhavenCode format_purity(WallhavenAPI *wa, int purity)
{
    if (!purity)
        return WALLHAVEN_OK;
    if (purity & NSFW)
        checkp_return(wa->apikey, WALLHAVEN_NO_API_KEY);

    char c[4] = {0};
    int ci = 0;
    for (int i = 2; i > -1; --i, purity >>= 1)
        c[i] = (purity & 1) ? '1' : '0';

#ifdef DEBUG
    printf("format_purity\n%s\n", c);
#endif

    check_return(append_query(wa, "purity", c), WALLHAVEN_CURL_FAIL);
    return WALLHAVEN_OK;
}

static WallhavenCode format_sorting(WallhavenAPI *wa, Sorting sorting)
{
    char *s;
    switch (sorting)
    {
    case 0:
        return WALLHAVEN_OK;
    case DATE_ADDED:
        s = "date_added";
        break;
    case RELEVANCE:
        s = "relevance";
        break;
    case RANDOM:
        s = "random";
        break;
    case VIEWS:
        s = "views";
        break;
    case FAVORITES:
        s = "favorites";
        break;
    case TOPLIST:
        s = "toplist";
        break;
    default:
        return WALLHAVEN_UNKOWN_SORTING;
    }

#ifdef DEBUG
    printf("format_sorting\n%s\n", s);
#endif

    check_return(append_query(wa, "sorting", s), WALLHAVEN_CURL_FAIL);
    return WALLHAVEN_OK;
}

static WallhavenCode format_order(WallhavenAPI *wa, Order order)
{
    char *s;
    switch (order)
    {
    case 0:
        return WALLHAVEN_OK;
    case DESCENDING:
        s = "desc";
        break;
    case ASCENDING:
        s = "asc";
        break;
    default:
        return WALLHAVEN_UNKOWN_ORDER;
    }

#ifdef DEBUG
    printf("format_order\n%s\n", s);
#endif

    check_return(append_query(wa, "order", s), WALLHAVEN_CURL_FAIL);
    return WALLHAVEN_OK;
}

static WallhavenCode format_toprange(WallhavenAPI *wa, TopRange toprange, Sorting sorting)
{
    if (sorting != TOPLIST && toprange)
        return WALLHAVEN_SORTING_SHOULD_BE_TOPLIST;
    char *s;
    switch (toprange)
    {
    case 0:
        return WALLHAVEN_OK;
    case ONE_DAY:
        s = "1d";
        break;
    case THREE_DAYS:
        s = "3d";
        break;
    case ONE_WEEK:
        s = "1w";
        break;
    case ONE_MONTH:
        s = "1M";
        break;
    case THREE_MONTHS:
        s = "3M";
        break;
    case SIX_MONTHS:
        s = "6M";
        break;
    case ONE_YEAR:
        s = "1y";
        break;
    default:
        return WALLHAVEN_UNKOWN_TOPRANGE;
    }

#ifdef DEBUG
    printf("format_toprange\n%s\n", s);
#endif

    check_return(append_query(wa, "topRange", s), WALLHAVEN_CURL_FAIL);
    return WALLHAVEN_OK;
}

static WallhavenCode format_atleast(WallhavenAPI *wa, const char *atleast)
{
    checkp_return(atleast, WALLHAVEN_OK);
    check_return(append_query(wa, "atleast", atleast), WALLHAVEN_CURL_FAIL);
    return WALLHAVEN_OK;
}

static WallhavenCode format_resolutions(WallhavenAPI *wa, const char *resolutions)
{
    checkp_return(resolutions, WALLHAVEN_OK);
    check_return(append_query(wa, "resolutions", resolutions), WALLHAVEN_CURL_FAIL);
    return WALLHAVEN_OK;
}

static WallhavenCode format_ratios(WallhavenAPI *wa, const char *ratios)
{
    checkp_return(ratios, WALLHAVEN_OK);
    check_return(append_query(wa, "ratios", ratios), WALLHAVEN_CURL_FAIL);
    return WALLHAVEN_OK;
}

static WallhavenCode format_colors(WallhavenAPI *wa, const char *colors)
{
    checkp_return(colors, WALLHAVEN_OK);
    check_return(append_query(wa, "colors", colors), WALLHAVEN_CURL_FAIL);
    return WALLHAVEN_OK;
}

static WallhavenCode format_page(WallhavenAPI *wa, const int page)
{
    if (!page)
        return WALLHAVEN_OK;

    size_t size = snprintf(NULL, 0, "%d", page) + 1;
    char *s = (char *)malloc(size);
    snprintf(s, size, "%d", page);

    WallhavenCode r = append_query(wa, "page", s);
    free(s);
    return r;
}

static WallhavenCode format_seed(WallhavenAPI *wa, const char *seed)
{
    checkp_return(seed[0], WALLHAVEN_OK);
    check_return(append_query(wa, "seed", seed), WALLHAVEN_CURL_FAIL);
    return WALLHAVEN_OK;
}

static CURLUcode reset(WallhavenAPI *wa)
{
    // Reset the queries and options
    curl_easy_reset(wa->curl);
    wa->api_key_set = false;
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

    wa->api_call_limit_error = default_api_call_limit;
    wa->response = NULL;
    wa->api_key_set = false;
    wa->apikey = NULL;
    wa->start_time = -1;

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
    wa->response = response;

    // Write curl output to response
    check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEFUNCTION, write_function), WALLHAVEN_CURL_FAIL);
    check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEDATA, (void *)response), WALLHAVEN_CURL_FAIL);

    return WALLHAVEN_OK;
}

WallhavenCode wallhaven_write_to_file(WallhavenAPI *wa, FILE *file)
{
    check_return(reset(wa), WALLHAVEN_CURL_FAIL);
    wa->response = NULL;

    // Write curl ouput to a file
    check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEFUNCTION, write_function_tofile), WALLHAVEN_CURL_FAIL);
    check_return(curl_easy_setopt(wa->curl, CURLOPT_WRITEDATA, (void *)file), WALLHAVEN_CURL_FAIL);

    return WALLHAVEN_OK;
}

WallhavenCode wallhaven_get_result(WallhavenAPI *wa, Path p, const char *id)
{
    size_t size;
    char *path;

    if (wa->response)
    {
        wa->response->size = 0;
        if (!wa->response->value)
            wa->response->value = NULL;
    }

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
    case SEARCH:
        if (wa->apikey)
            check_return(append_query(wa, "apikey", wa->apikey), WALLHAVEN_CURL_FAIL);
        size = 1 + strlen(SEARCH_PATH);
        path = (char *)malloc(size);
        snprintf(path, size, SEARCH_PATH);
        break;
    case COLLECTIONS:
        if (!id)
        {
            checkp_return(wa->apikey, WALLHAVEN_NO_API_KEY);
            check_return(append_query(wa, "apikey", wa->apikey), WALLHAVEN_CURL_FAIL);
        }
        size = 1 + strlen(COLLECTIONS_PATH) + (id ? strlen(id) + 1 /*For '/' */ : 0);
        path = (char *)malloc(size);
        snprintf(path, size, COLLECTIONS_PATH "/%s", id);
        break;
    default:
        return WALLHAVEN_UNKNOW_PATH;
    }

    check_return(curl_url_set(wa->url, CURLUPART_PATH, path, CURLU_URLENCODE), WALLHAVEN_CURL_FAIL);
    check_return(curl_easy_setopt(wa->curl, CURLOPT_CURLU, wa->url), WALLHAVEN_CURL_FAIL);

    if (wa->start_time == -1)
        time(&wa->start_time);

    if (difftime(time(NULL), wa->start_time) > 60)
        time(&wa->start_time);

    CURLcode c = curl_easy_perform(wa->curl);

    long response_code;
    check_return(curl_easy_getinfo(wa->curl, CURLINFO_RESPONSE_CODE, &response_code), WALLHAVEN_CURL_FAIL);
    if (response_code == 429)
    {
        if (wa->api_call_limit_error(&wa->start_time))
            return wallhaven_get_result(wa, p, id);
        else
            return WALLHAVEN_TOO_MANY_REQUSTS_ERROR;
    }
    else if (response_code == 401)
        return WALLHAVEN_UNAUTHORIZED_ERROR;

#ifdef DEBUG
    curl_url_get(wa->url, CURLUPART_URL, &path, 0);
    printf("URL: %s\n", path);
    printf("CURLcode: %d\n", c);
    printf("Response code: %ld\n", response_code);
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
        "\t\tUser name = %s\n"
        "\t\tId = %s\n"
        "\t\tType = %d\n"
        "\t\tlike = %s\n"
        "\tCategories = %d\n"
        "\tPurity = %d\n"
        "\tSorting = %d\n"
        "\tOrder = %d\n"
        "\tTopRange = %d\n"
        "\tAtleast = %s\n"
        "\tResolutions = %s\n"
        "\tratios = %s\n"
        "\tColors = %s\n"
        "\tPage = %d\n"
        "\tSeed = %s\n",
        p->q->tags,
        p->q->user_name,
        p->q->id,
        p->q->type,
        p->q->like,
        p->categories,
        p->purity,
        p->sorting,
        p->order,
        p->toprange,
        p->atleast,
        p->resolutions,
        p->ratios,
        p->colors,
        p->page,
        p->seed);
#endif

    WallhavenCode wc;

    wc = format_q(wa, p->q);
    check_return(wc, wc);

    wc = format_categories(wa, p->categories);
    check_return(wc, wc);

    wc = format_purity(wa, p->purity);
    check_return(wc, wc);

    wc = format_sorting(wa, p->sorting);
    check_return(wc, wc);

    wc = format_order(wa, p->order);
    check_return(wc, wc);

    wc = format_toprange(wa, p->toprange, p->sorting);
    check_return(wc, wc);

    wc = format_atleast(wa, p->atleast);
    check_return(wc, wc);

    wc = format_resolutions(wa, p->resolutions);
    check_return(wc, wc);

    wc = format_ratios(wa, p->ratios);
    check_return(wc, wc);

    wc = format_colors(wa, p->colors);
    check_return(wc, wc);

    wc = format_page(wa, p->page);
    check_return(wc, wc);

    wc = format_seed(wa, p->seed);
    check_return(wc, wc);

    return wallhaven_get_result(wa, SEARCH, NULL);
}

WallhavenCode wallhaven_wallpapers_of_collections(WallhavenAPI *wa, const char *user, const char *id, int purity)
{
    size_t size = snprintf(NULL, 0, "%s/%s", user, id) + 1;
    char *concat = (char *)malloc(size);
    snprintf(concat, size, "%s/%s", user, id);

    WallhavenCode wc = format_purity(wa, purity);
    check_return(wc, wc);

    return wallhaven_get_result(wa, COLLECTIONS, concat);
}

void wallhaven_set_on_api_call_limit_error(WallhavenAPI *wa, onMaxAPICallLimitError func)
{
    wa->api_call_limit_error = func;
}
