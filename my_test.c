#include <stdio.h>
#include <stdlib.h>

#include "wallhavenapi.h"

// To test whether things works or not

int main()
{
    WallhavenAPI *wa = wallhaven_init();
#ifdef APIKEY
    wallhaven_apikey(wa, APIKEY);
#endif

    // Wallpaper info
    FILE *file = fopen("./test.json", "w");
    if (!(file))
        return 1;
    Response r = {0};
    wallhaven_write_to_response(wa, &r);
    WallhavenCode c = wallhaven_wallpaper_info(wa, "3lepy9");
    wallhaven_write_to_file(wa, file);
    // printf("%s\n", r.value);
    wallhaven_wallpaper_info(wa, "3lepy9");
    free(r.value);
    fclose(file);

    // Tag info
    file = fopen("./test1.json", "w");
    if (!(file))
        return 2;
    Response rt = {0};
    wallhaven_write_to_response(wa, &rt);
    WallhavenCode ct = wallhaven_tag_info(wa, "77513");
    // printf("%s\n", rt.value);
    wallhaven_write_to_file(wa, file);
    wallhaven_tag_info(wa, "77513");
    free(rt.value);
    fclose(file);

    // User settings
    file = fopen("./test2.json", "w");
    if (!(file))
        return 3;
    Response rs = {0};
    wallhaven_write_to_response(wa, &rs);
    WallhavenCode cs = wallhaven_user_settings(wa);
    if (cs == WALLHAVEN_OK)
    {
        // printf("%s\n", rs.value);
        wallhaven_write_to_file(wa, file);
        wallhaven_user_settings(wa);
    }
    else
    {
        printf("No API key is provided\n");
    }
    free(rs.value);
    fclose(file);

    // Search
    Parameters p = (Parameters){
        .q = &(Query){
            0
            // .tags = "Tag -Exclude +Include",
            // .user_name = "user name",
            // .id = "id",
            // .type = PNG, // JPG, JPEG, PNG
            // .like = "like"
        },
        // .categories = GENERAL | ANIME | PEOPLE,
        // .purity = SKETCHY | SFW | NSFW,
        // .sorting = TOPLIST, // DATE_ADDED, RELEVANCE,RANDOM,VIEWS,FAVORITES,TOPLIST
        // .order = DESCENDING,  // ASCENDING, DESCENDING
        // .toprange = ONE_YEAR, // ONE_DAY, THREE_DAYS, ONE_WEEK, ONE_MONTH, THREE_MONTHS, SIX_MONTHS, ONE_YEAR
        // .atleast = "wxh",
        // .resolutions = "w1xh1,w2xh2",
        // .ratios = "9x16,10x16",
        // .colors = Rosewood "," GreenLeaf, // Don't want to paste all possibe values here :)
        // .page = 100,
        // .seed = "azAZ09"
    };

    file = fopen("./test3.json", "w");
    wallhaven_write_to_file(wa, file);
    WallhavenCode wc = wallhaven_search(wa, &p);
    if (wc == WALLHAVEN_NO_API_KEY)
        printf("Api key is not given\n");
    else if (wc == WALLHAVEN_USING_ID_IN_COMBINATION)
        printf("Used id in a combination\n");
    else if (wc == WALLHAVEN_SORTING_SHOULD_BE_TOPLIST)
        printf("Sorting is not toplist\n");
    fclose(file);

    wallhaven_free(wa);
}
