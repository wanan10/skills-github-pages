#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void *xrealloc(void *ptr, size_t size) {
	void *p = realloc(ptr, size);
	if (!p) {
		fprintf(stderr, "Out of memory (realloc %zu)\n", size);
		exit(1);
	}
	return p;
}

void intvec_init(IntVec *v) {
	v->data = NULL;
	v->size = 0;
	v->capacity = 0;
}

void intvec_free(IntVec *v) {
	free(v->data);
	v->data = NULL;
	v->size = v->capacity = 0;
}

void intvec_clear(IntVec *v) {
	v->size = 0;
}

void intvec_push(IntVec *v, int value) {
	if (v->size == v->capacity) {
		size_t newcap = v->capacity ? v->capacity * 2 : 16;
		v->data = (int*)xrealloc(v->data, newcap * sizeof(int));
		v->capacity = newcap;
	}
	v->data[v->size++] = value;
}

void intptrvec_init(IntPtrVec *v) {
	v->data = NULL;
	v->size = 0;
	v->capacity = 0;
}

void intptrvec_free(IntPtrVec *v) {
	free(v->data);
	v->data = NULL;
	v->size = v->capacity = 0;
}

void intptrvec_push(IntPtrVec *v, int *ptr) {
	if (v->size == v->capacity) {
		size_t newcap = v->capacity ? v->capacity * 2 : 16;
		v->data = (int**)xrealloc(v->data, newcap * sizeof(int*));
		v->capacity = newcap;
	}
	v->data[v->size++] = ptr;
}

char *read_file_all(const char *path, size_t *out_len) {
	FILE *f = fopen(path, "rb");
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	if (len < 0) { fclose(f); return NULL; }
	rewind(f);
	char *buf = (char*)malloc((size_t)len + 1);
	if (!buf) { fclose(f); return NULL; }
	size_t n = fread(buf, 1, (size_t)len, f);
	fclose(f);
	buf[n] = '\0';
	if (out_len) *out_len = n;
	return buf;
}

int write_file_all(const char *path, const char *data, size_t len) {
	FILE *f = fopen(path, "wb");
	if (!f) return -1;
	size_t n = fwrite(data, 1, len, f);
	fclose(f);
	return n == len ? 0 : -1;
}

