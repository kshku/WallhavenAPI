bool default_api_call_limit(time_t *start_time)
{
    time_t t = time(NULL);
    int wait_time = 60 - difftime(t, *start_time) + 1;
    if (wait_time > 60)
        return true;
    printf("Hit max api call limit, waiting for %d seconds before retrying...\n", wait_time);
    sleep_ms(wait_time * 1000); // Portable sleep function

    return true;
}
