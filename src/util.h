#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

typedef struct {
	int *data;
	size_t size;
	size_t capacity;
} IntVec;

void intvec_init(IntVec *v);
void intvec_free(IntVec *v);
void intvec_clear(IntVec *v);
void intvec_push(IntVec *v, int value);

typedef struct {
	int **data;
	size_t size;
	size_t capacity;
} IntPtrVec;

void intptrvec_init(IntPtrVec *v);
void intptrvec_free(IntPtrVec *v);
void intptrvec_push(IntPtrVec *v, int *ptr);

char *read_file_all(const char *path, size_t *out_len);
int write_file_all(const char *path, const char *data, size_t len);

#endif

