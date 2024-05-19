#include "wallhavenapi.h"
#include <stdlib.h>

int main()
{
    Response r = {0};

    WallhavenAPI *wa = wallhaven_init();
    wallhaven_write_to_response(wa, &r);

    // Haven't included all the parameters
    wallhaven_search(wa, &(Parameters){
                             .q = &(Query){
                                 .tags = "nature -forest +waterfall",
                                 .type = PNG,
                             },
                             .atleast = "1920x1080",
                             .categories = GENERAL | ANIME,
                             .purity = SFW,
                             .order = DESCENDING,
                             .seed = "azAZ09", // Sometimes giving seven characters doesn't throw any error, Use only six characters
                             .colors = Rosewood "," GunPowder,
                             .page = 2,
                         });
    printf("%s\n", r.value);

    free(r.value);
    wallhaven_free(wa);
}
