#ifndef WALLHAVEN_API_H
#define WALLHAVEN_API_H

#include <stdio.h>

// Error codes returned by functions
typedef enum
{
    WALLHAVEN_OK,
    WALLHAVEN_CURL_FAIL,
    WALLHAVEN_FAIL
} WallhavenCode;

// Struct to store the response
// Needs to call free(response.value) after using it
typedef struct
{
    char *value;
    size_t size;
} Response;

// Defined in c file
typedef struct WallhavenAPI WallhavenAPI;

// Initializer function
WallhavenAPI *wallhaven_init();
// Terminator function
void wallhaven_free(WallhavenAPI *wa);

// Provide the api key
WallhavenCode wallhaven_apikey(WallhavenAPI *wa, const char *apikey);

// Write wallpaper info to Response
WallhavenCode wallhaven_wallpaper_info(WallhavenAPI *wa, const char *id, Response *response);
// Write wallpaper info to a file
WallhavenCode wallhaven_wallpaper_info_tofile(WallhavenAPI *wa, const char *id, FILE *file);

#endif
