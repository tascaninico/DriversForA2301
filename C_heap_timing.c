#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdint.h>



long long get_ns_diff(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1e9 + (end.tv_nsec - start.tv_nsec);
}

int main() {
    struct timespec start, before_free, after_alloc, after_free;

   
    clock_gettime(CLOCK_MONOTONIC, &start);
    uint8_t *data = (uint8_t *)malloc(1000 * sizeof(uint8_t));
    clock_gettime(CLOCK_MONOTONIC, &after_alloc);

    for (int i = 0; i < 5; ++i) data[i] = i;

    clock_gettime(CLOCK_MONOTONIC, &before_free);
    free(data);
    clock_gettime(CLOCK_MONOTONIC, &after_free);

    printf("malloc took %lld ns\n", get_ns_diff(start, after_alloc));
    printf("free took %lld ns\n", get_ns_diff(before_free, after_free));

    return 0;
}
