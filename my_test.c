#include <stdio.h>
#include <stdlib.h>

#include "wallhavenapi.h"

// To test whether things works or not

void print_and_reset(Response *response)
{
    printf("%s\n", response->value);
    response->size = 0;
}

int main()
{
    WallhavenAPI *wa = wallhaven_init();
#ifdef APIKEY
    wallhaven_apikey(wa, APIKEY);
#endif
    Response response = {0};
    wallhaven_write_to_response(wa, &response);

    /* wallhaven_my_collections(wa);
     print_and_reset(&response);

     wallhaven_collections_of(wa, "Yukinoshita");
     print_and_reset(&response);

     // Just hack
     wallhaven_collections_of(wa, "Yukinoshita/854694");
     print_and_reset(&response);

     wallhaven_wallpapers_of_collections(wa, "Yukinoshita", "854694", 0);
     print_and_reset(&response);

     for (int i = 0; i < 50; ++i)
     {
         printf("Iteration %d\n", i + 1);
         wallhaven_wallpaper_info(wa, "3lepy9");
         print_and_reset(&response);
     }

     // Wallpaper info
     FILE *test = fopen("./test.json", "w");
     if (!(test))
         return 1;
     WallhavenCode c = wallhaven_wallpaper_info(wa, "3lepy9");
     print_and_reset(&response);
     wallhaven_write_to_file(wa, test);
     wallhaven_wallpaper_info(wa, "3lepy9");
     fclose(test);

     // Tag info
     FILE *test1 = fopen("./test1.json", "w");
     if (!(test1))
         return 2;
     WallhavenCode ct = wallhaven_tag_info(wa, "77513");
     print_and_reset(&response);
     wallhaven_write_to_file(wa, test1);
     wallhaven_tag_info(wa, "77513");
     fclose(test1);

     // User settings
     FILE *test2 = fopen("./test2.json", "w");
     if (!(test2))
         return 3;
     WallhavenCode cs = wallhaven_user_settings(wa);
     if (cs == WALLHAVEN_OK)
     {
         print_and_reset(&response);
         wallhaven_write_to_file(wa, test2);
         wallhaven_user_settings(wa);
     }
     else
     {
         printf("No API key is provided\n");
     }
     fclose(test2);
 */
    // Search
    Parameters p = (Parameters){
        .q = &(Query){
            0,
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
        .seed = "azAZ09a",
    };

    FILE *test3 = fopen("./test3.json", "w");
    if (!test3)
        return 4;
    wallhaven_write_to_file(wa, test3);
    WallhavenCode wc = wallhaven_search(wa, &p);
    if (wc == WALLHAVEN_NO_API_KEY)
        printf("Api key is not given\n");
    else if (wc == WALLHAVEN_USING_ID_IN_COMBINATION)
        printf("Used id in a combination\n");
    else if (wc == WALLHAVEN_SORTING_SHOULD_BE_TOPLIST)
        printf("Sorting is not toplist\n");
    fclose(test3);

    wallhaven_free(wa);
    free(response.value);
}
