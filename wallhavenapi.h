/**
 * @mainpage [Wallhaven](https://wallhaven.cc)'s API implemenntation in C
 * @brief Implemented the wallhaven api for fun
 * @note Read [API documentation](https://wallhaven.cc/help/api) before using
 *
 */

/**
 * @file wallhavenapi.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-05-18
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef WALLHAVEN_API_H
#define WALLHAVEN_API_H

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include <curl/curl.h>

/**
 * @defgroup colors Color macros
 * @brief Defining the hex values for the colors
 *
 * Color names are taken from from [ArtyClick Colors](https://colors.artyclick.com/color-name-finder/)
 * @{
 */
/** @brief Name for the color hex 660000 */
#define Rosewood "660000"
/** @brief Name for the color hex 990000 */
#define CrimsonRed "990000"
/** @brief Name for the color hex cc0000 */
#define RossoCorsa "cc0000"
/** @brief Name for the color hex cc3333 */
#define PersianRed "cc3333"
/** @brief Name for the color hex ea4c88 */
#define DarkPink "ea4c88"
/** @brief Name for the color hex 993399 */
#define WarmPurple "993399"
/** @brief Name for the color hex 663399 */
#define Eminence "663399"
/** @brief Name for the color hex 333399 */
#define Blueberry "333399"
/** @brief Name for the color hex 0066cc */
#define ScienceBlue "0066cc"
/** @brief Name for the color hex 0099cc */
#define PacificBlue "0099cc"
/** @brief Name for the color hex 66cccc */
#define Downy "66cccc"
/** @brief Name for the color hex 77cc33 */
#define AppleGreen "77cc33"
/** @brief Name for the color hex 669900 */
#define VenomGreen "669900"
/** @brief Name for the color hex 336600 */
#define GreenLeaf "336600"
/** @brief Name for the color hex 666600 */
#define GreenyBrown "666600"
/** @brief Name for the color hex 999900 */
#define BrownYellow "999900"
/** @brief Name for the color hex cccc33 */
#define BirdFlower "cccc33"
/** @brief Name for the color hex ffff00 */
#define ArtyClickYellow "ffff00"
/** @brief Name for the color hex ffcc33 */
#define Sunglow "ffcc33"
/** @brief Name for the color hex ff9900 */
#define OrangePeel "ff9900"
/** @brief Name for the color hex ff6600 */
#define BlazeOrange "ff6600"
/** @brief Name for the color hex cc6633 */
#define TerraCotta "cc6633"
/** @brief Name for the color hex 996633 */
#define Wood "996633"
/** @brief Name for the color hex 663300 */
#define NutmegWood "663300"
/** @brief Name for the color hex 000000 */
#define Black "000000"
/** @brief Name for the color hex 999999 */
#define LemonGrass "999999"
/** @brief Name for the color hex cccccc */
#define PastelGrey "cccccc"
/** @brief Name for the color hex ffffff */
#define White "ffffff"
/** @brief Name for the color hex 424153 */
#define GunPowder "424153"
/**@}*/

/**
 * @brief Get wallpaper information
 * @param wallhaven_api Pointer to the WallhavenAPI struct
 * @param id Id of the wallpaper to get information about
 *
 */
#define wallhaven_wallpaper_info(wallhaven_api, id) \
    wallhaven_get_result(wa, WALLPAPER_INFO, id)

/**
 * @brief Get Tag information
 * @param wallhaven_api Pointer to the WallhavenAPI struct
 * @param id Id of the tag to get information about
 *
 */
#define wallhaven_tag_info(wallhaven_api, id) \
    wallhaven_get_result(wa, TAG_INFO, id)

/**
 * @brief Get user settings from wallhaven
 * @param wallhaven_api Pointer to the WallhavenAPI struct
 * @note Should set the API key before using this function
 *
 */
#define wallhaven_user_settings(wallhaven_api) \
    wallhaven_get_result(wa, SETTINGS, NULL)

/**
 * @brief Get personal collections
 * @param wallhaven_api Pointer to the WallhavenAPI struct
 * @note Should set the API key before using this function
 *
 */
#define wallhaven_my_collections(wallhaven_api) \
    wallhaven_get_result(wallhaven_api, COLLECTIONS, NULL)

/**
 * @brief Get collections of another user
 * @param wallhaven_api Pointer to the WallhavenAPI struct
 * @param user_name Name of the user whose collections to get
 *
 */
#define wallhaven_collections_of(wallhaven_api, user_name) \
    wallhaven_get_result(wallhaven_api, COLLECTIONS, user_name)

/**
 * @brief Type of function to call when hit maximum API call limit
 * @param start_time Pointer to the  time at which the calls to API started
 * @return Return true if you want to retry
 * @return If return false, wallhave_get_result will return WALLHAVEN_TOO_MANY_REQUSTS_ERROR
 *
 */
typedef bool (*onMaxAPICallLimitError)(time_t *start_time);

/**
 * @brief Error codes returned from API functions
 *
 */
typedef enum
{
    WALLHAVEN_OK,                        /**< No Error */
    WALLHAVEN_CURL_FAIL,                 /**< Something went wrong with the curl */
    WALLHAVEN_NO_API_KEY,                /**< API key was not set, but required */
    WALLHAVEN_USING_ID_IN_COMBINATION,   /**< In query id is for exact tag search and cannot be combined (look at the [documentation](https://wallhaven.cc/help/api#search)) */
    WALLHAVEN_UNKNOW_PATH,               /**< The path given to the wallhaven_get_result function is unkown */
    WALLHAVEN_UNKOWN_SORTING,            /**< In Parameters given value given for sorting is unkown */
    WALLHAVEN_UNKOWN_ORDER,              /**< In Parameters given value for order is unkown */
    WALLHAVEN_UNKOWN_TOPRANGE,           /**< In Parameters given value for toprange is unkown */
    WALLHAVEN_SORTING_SHOULD_BE_TOPLIST, /**< To use Top Range, sorting must be TOPLIST (look at the [documentation](https://wallhaven.cc/help/api#search)) */
    WALLHAVEN_TOO_MANY_REQUSTS_ERROR,    /**< Returned when Maximum API call limit is hit and didn't retried to get the content */
    WALLHAVEN_UNAUTHORIZED_ERROR,        /**< Returned when API key is not correct or trying to access nsfw wallpapers without API key ([documentation](https://wallhaven.cc/help/api#limits)) */
} WallhavenCode;

/**
 * @brief Structure to store the response
 * @note Functions are not provided to maintain this struct. User have to manually handle the memory.
 *
 */
typedef struct
{
    char *value; /**< @brief response string */
    size_t size; /**< @brief size of the value */
} Response;
/**
 * @example response.c
 * @brief Example of using Response
 */

/**
 * @brief Enum for image format type
 *
 * Here JPG and JPEG are equivalent as documentation specifies
 *
 */
typedef enum
{
    PNG = 1,
    JPEG,
    JPG = JPEG
} Type;

/**
 * @brief Search query struct
 *
 * Struct to store the query values (first value in the Parameters list in [documentation](https://wallhaven.cc/help/api#search))
 *
 */
typedef struct
{
    char *tags;      /**< @brief Search fuzzily for a tag. Can include +tag (include a tag) and -tag (exclude a tag) also */
    char *user_name; /**< @brief Name fo the user whose uploads to search for */
    char *id;        /**< @brief Id of the tag to search for the wallpapers having the same tag (cannot be combined) */
    Type type;       /**< @brief Type of the images */
    char *like;      /**< @brief Id of the wallpaper to find the wallpapers of same kind */
} Query;

/**
 * @brief Enum representing the catgories of images
 *
 */
typedef enum
{
    PEOPLE = 1 << 0,
    ANIME = 1 << 1,
    GENERAL = 1 << 2
} Category;

/**
 * @brief Enum for purity of images
 *
 */
typedef enum
{
    NSFW = 1 << 0,
    SKETCHY = 1 << 1,
    SFW = 1 << 2
} Purity;

/**
 * @brief Enum for sorting ways of the images
 *
 */
typedef enum
{
    DATE_ADDED = 1,
    RELEVANCE,
    RANDOM,
    VIEWS,
    FAVORITES,
    TOPLIST
} Sorting;

/**
 * @brief Enum for which order the images to be sorted
 *
 */
typedef enum
{
    DESCENDING = 1,
    ASCENDING
} Order;

/**
 * @brief Enum for Toprange values
 *
 */
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

/**
 * @brief All parameters to search images
 *
 * This struct is passed to the wallhaven_search function specifying the parameter
 *
 * [Documentation of the pararmeters](https://wallhaven.cc/help/api#search)
 *
 */
typedef struct Parameters
{
    Query *q;          /**< @brief Pointer to the Query structure to use */
    int categories;    /**< @brief Categories flag. Search for the images belonging to the categories */
    int purity;        /**< @brief Purity flag. Search for the images having the purity */
    Sorting sorting;   /**< @brief Method to sort the images */
    Order order;       /**< @brief Sorting order of the images */
    TopRange toprange; /**< @brief Toplist in the range of */
    char *atleast;     /**< @brief The minimum resolution the image should have  */
    char *resolutions; /**< @brief List of exact wallpaper resolutions to look for */
    char *ratios;      /**< @brief List of exact wallpaper ratios to look for */
    char *colors;      /**< @brief Search for the images having the colors */
    int page;          /**< @brief List images in the page number */
    char seed[6 + 1];  /**< @brief Seed for random results */
} Parameters;
/**
 * @example parameters.c
 * @brief Example of using parametrs
 *
 */

typedef enum
{
    WALLPAPER_INFO,
    TAG_INFO,
    SETTINGS,
    SEARCH,
    COLLECTIONS,
} Path;

// Defined in c file
typedef struct WallhavenAPI
{
    CURL *curl;
    CURLU *url;
    const char *apikey;
    bool api_key_set;
    onMaxAPICallLimitError api_call_limit_error;
    time_t start_time;
} WallhavenAPI;

// Wallhaven api functions

// Initializer function
WallhavenAPI *wallhaven_init();
// Terminator function
void wallhaven_free(WallhavenAPI *wa);

// Provide the api key
void wallhaven_apikey(WallhavenAPI *wa, const char *apikey);

// Write the result to Response when called wallhaven_get_result
WallhavenCode wallhaven_write_to_response(WallhavenAPI *wa, Response *response);

// Write the result to a file when called wallhaven_get_result
WallhavenCode wallhaven_write_to_file(WallhavenAPI *wa, FILE *file);

// Get the result
// If neither wallhaven_write_to_response nor wallhaven_write_to_file set writes result to stdout
WallhavenCode wallhaven_get_result(WallhavenAPI *wa, Path p, const char *id);

// Search wallpapers
WallhavenCode wallhaven_search(WallhavenAPI *wa, Parameters *p);

// Get wallpapers in the collection
WallhavenCode wallhaven_wallpapers_of_collections(WallhavenAPI *wa, const char *user, const char *id, int purity);

// Set function to call on maximum api call limit is hit
void wallhaven_set_on_api_call_limit_error(WallhavenAPI *wa, onMaxAPICallLimitError func);

#endif
