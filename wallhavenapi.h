#ifndef WALLHAVEN_API_H
#define WALLHAVEN_API_H

// NOTE: Read api before using. API LINk: https://wallhaven.cc/help/api

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

// Color names are from https://colors.artyclick.com/color-name-finder/
#define Rosewood "660000"
#define CrimsonRed "990000"
#define RossoCorsa "cc0000"
#define PersianRed "cc3333"
#define DarkPink "ea4c88"
#define WarmPurple "993399"
#define Eminence "663399"
#define Blueberry "333399"
#define ScienceBlue "0066cc"
#define PacificBlue "0099cc"
#define Downy "66cccc"
#define AppleGreen "77cc33"
#define VenomGreen "669900"
#define GreenLeaf "336600"
#define GreenyBrown "666600"
#define BrownYellow "999900"
#define BirdFlower "cccc33"
#define ArtyClickYellow "ffff00"
#define Sunglow "ffcc33"
#define OrangePeel "ff9900"
#define BlazeOrange "ff6600"
#define TerraCotta "cc6633"
#define Wood "996633"
#define NutmegWood "663300"
#define Black "000000"
#define LemonGrass "999999"
#define PastelGrey "cccccc"
#define White "ffffff"
#define GunPowder "424153"

// Get wallpaper info
#define wallhaven_wallpaper_info(wallhaven_api, id) \
    wallhaven_get_result(wa, WALLPAPER_INFO, id)

// Get tag info
#define wallhaven_tag_info(wallhaven_api, id) \
    wallhaven_get_result(wa, TAG_INFO, id)

// Get user settings
// Should set the apikey before this using wallhaven_apikey
#define wallhaven_user_settings(wallhaven_api) \
    wallhaven_get_result(wa, SETTINGS, NULL)

// Get personal collections
#define wallhaven_my_collections(wallhaven_api) \
    wallhaven_get_result(wallhaven_api, COLLECTIONS, NULL)

// Get collections of another user
#define wallhaven_collections_of(wallhaven_api, user_name) \
    wallhaven_get_result(wallhaven_api, COLLECTIONS, user_name)

// Return true to retry
// Else wallhaven_get_result will return WALLHAVEN_TOO_MANY_REQUESTS_ERROR
typedef bool (*onMaxAPICallLimit)(time_t *start_time);

// Error codes returned by functions
typedef enum
{
    WALLHAVEN_OK,
    WALLHAVEN_CURL_FAIL,
    WALLHAVEN_FAIL,
    WALLHAVEN_NO_API_KEY,
    WALLHAVEN_USING_ID_IN_COMBINATION,
    WALLHAVEN_UNKNOW_PATH,
    WALLHAVEN_UNKOWN_SORTING,
    WALLHAVEN_UNKOWN_ORDER,
    WALLHAVEN_UNKOWN_TOPRANGE,
    WALLHAVEN_SORTING_SHOULD_BE_TOPLIST,
    WALLHAVEN_TOO_MANY_REQUSTS_ERROR,
    WALLHAVEN_UNAUTHORIZED_ERROR,
} WallhavenCode;

// Struct to store the response
// Needs to call free(response.value) after using it
typedef struct
{
    char *value;
    size_t size;
} Response;

// To check enum values as if(enumvalue) we will start from 1

typedef enum
{
    PNG = 1,
    JPEG,
    JPG = JPEG
} Type;

/*
tagname - search fuzzily for a tag/keyword.
-tagname - exclude a tag/keyword.
+tag1 +tag2 - must have tag1 and tag2.
+tag1 -tag2 - must have tag1 and NOT tag2.
@username - user uploads.
id:123 - Exact tag search (can not be combined).
type:{png/jpg} - Search for file type (jpg = jpeg).
like : wallpaper ID - Find wallpapers with similar tags.
*/
typedef struct
{
    char *tags;
    char *user_name;
    char *id;
    Type type;
    char *like;
} Query;

typedef enum
{
    PEOPLE = 1 << 0,
    ANIME = 1 << 1,
    GENERAL = 1 << 2
} Category;

typedef enum
{
    NSFW = 1 << 0,
    SKETCHY = 1 << 1,
    SFW = 1 << 2
} Purity;

typedef enum
{
    DATE_ADDED = 1,
    RELEVANCE,
    RANDOM,
    VIEWS,
    FAVORITES,
    TOPLIST
} Sorting;

typedef enum
{
    DESCENDING = 1,
    ASCENDING
} Order;

typedef enum
{
    ONE_DAY = 1,
    THREE_DAYS,
    ONE_WEEK,
    ONE_MONTH,
    THREE_MONTHS,
    SIX_MONTHS,
    ONE_YEAR
} TopRange;

// Parameter structure for sending to search function
typedef struct Parameters
{
    Query *q;
    int categories;
    int purity;
    Sorting sorting;
    Order order;
    TopRange toprange;
    char *atleast;
    char *resolutions;
    char *ratios;
    char *colors;
    int page; // If set to zero ignores
    char seed[6 + 1];
} Parameters;

typedef enum
{
    WALLPAPER_INFO,
    TAG_INFO,
    SETTINGS,
    SEARCH,
    COLLECTIONS,
} Path;

// Defined in c file
typedef struct WallhavenAPI WallhavenAPI;

// Wallhaven api functions

// Initializer function
WallhavenAPI *wallhaven_init();
// Terminator function
void wallhaven_free(WallhavenAPI *wa);

// Provide the api key
WallhavenCode wallhaven_apikey(WallhavenAPI *wa, const char *apikey);

// Write the result to Response when called wallhaven_get_result
WallhavenCode wallhaven_write_to_response(WallhavenAPI *wa, Response *response);

// Write the result to a file when called wallhaven_get_result
WallhavenCode wallhaven_write_to_file(WallhavenAPI *wa, FILE *file);

// Get the result
// If neither wallhaven_write_to_response nor wallhaven_write_to_file set writes result to stdout
WallhavenCode wallhaven_get_result(WallhavenAPI *wa, Path p, const char *id);

// Search wallpapers
WallhavenCode wallhaven_search(WallhavenAPI *wa, const Parameters *p);

// Get wallpapers in the collection
WallhavenCode wallhaven_wallpapers_of_collections(WallhavenAPI *wa, const char *user, const char *id, int purity);

#endif
