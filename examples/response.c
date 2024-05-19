#include "wallhavenapi.h"
#include <stdlib.h>

void main()
{
    // Create the response
    Response response = {0};

    WallhavenAPI *wa = wallhaven_init();

    // Tell wallhaven to write to the reponse
    wallhaven_write_to_response(wa, &response);

    // Make a API call
    wallhaven_tag_info(wa, "1");

    // Get the reponse
    printf("%s\n", response.value);

    // For multiple use, to override the previous value
    response.size = 0; // if size is not zero, next response will be appended
    wallhaven_tag_info(wa, "2");
    printf("%s\n", response.value);

    // At the end free the response.value
    free(response.value);

    wallhaven_free(wa);
}
