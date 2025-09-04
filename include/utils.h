#ifndef UTILS_H
#define UTILS_H

#include <time.h>

// Return current monotonic time in milliseconds
static inline long long time_now_ms(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (long long)ts.tv_sec * 1000LL + (long long)(ts.tv_nsec / 1000000LL);
}

static inline long long time_elapsed_ms(long long start_ms) {
	return time_now_ms() - start_ms;
}

// Sign helpers
static inline int lit_var(int lit) { return lit > 0 ? lit : -lit; }
static inline int lit_sign(int lit) { return lit > 0 ? 1 : -1; }

#endif // UTILS_H
