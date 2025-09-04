#include "sat_solver_c.h"

// ===== 整数动态数组实现 =====
IntArray* intArrayCreate() {
    IntArray* arr = (IntArray*)malloc(sizeof(IntArray));
    if (!arr) return NULL;
    
    arr->data = (int*)malloc(INITIAL_CAPACITY * sizeof(int));
    if (!arr->data) {
        free(arr);
        return NULL;
    }
    
    arr->size = 0;
    arr->capacity = INITIAL_CAPACITY;
    return arr;
}

void intArrayDestroy(IntArray* arr) {
    if (arr) {
        if (arr->data) {
            free(arr->data);
        }
        free(arr);
    }
}

static void intArrayResize(IntArray* arr) {
    int newCapacity = arr->capacity * 2;
    int* newData = (int*)realloc(arr->data, newCapacity * sizeof(int));
    if (newData) {
        arr->data = newData;
        arr->capacity = newCapacity;
    }
}

void intArrayPush(IntArray* arr, int value) {
    if (arr->size >= arr->capacity) {
        intArrayResize(arr);
    }
    arr->data[arr->size++] = value;
}

int intArrayGet(IntArray* arr, int index) {
    if (index >= 0 && index < arr->size) {
        return arr->data[index];
    }
    return 0; // 默认值
}

void intArrayPop(IntArray* arr) {
    if (arr->size > 0) {
        arr->size--;
    }
}

void intArrayClear(IntArray* arr) {
    arr->size = 0;
}

bool intArrayEmpty(IntArray* arr) {
    return arr->size == 0;
}

// ===== 文字动态数组实现 =====
LiteralArray* literalArrayCreate() {
    LiteralArray* arr = (LiteralArray*)malloc(sizeof(LiteralArray));
    if (!arr) return NULL;
    
    arr->data = (Literal*)malloc(INITIAL_CAPACITY * sizeof(Literal));
    if (!arr->data) {
        free(arr);
        return NULL;
    }
    
    arr->size = 0;
    arr->capacity = INITIAL_CAPACITY;
    return arr;
}

void literalArrayDestroy(LiteralArray* arr) {
    if (arr) {
        if (arr->data) {
            free(arr->data);
        }
        free(arr);
    }
}

static void literalArrayResize(LiteralArray* arr) {
    int newCapacity = arr->capacity * 2;
    Literal* newData = (Literal*)realloc(arr->data, newCapacity * sizeof(Literal));
    if (newData) {
        arr->data = newData;
        arr->capacity = newCapacity;
    }
}

void literalArrayPush(LiteralArray* arr, Literal lit) {
    if (arr->size >= arr->capacity) {
        literalArrayResize(arr);
    }
    arr->data[arr->size++] = lit;
}

Literal literalArrayGet(LiteralArray* arr, int index) {
    if (index >= 0 && index < arr->size) {
        return arr->data[index];
    }
    Literal empty = {0, true};
    return empty;
}

// ===== 子句动态数组实现 =====
ClauseArray* clauseArrayCreate() {
    ClauseArray* arr = (ClauseArray*)malloc(sizeof(ClauseArray));
    if (!arr) return NULL;
    
    arr->data = (Clause*)malloc(INITIAL_CAPACITY * sizeof(Clause));
    if (!arr->data) {
        free(arr);
        return NULL;
    }
    
    arr->size = 0;
    arr->capacity = INITIAL_CAPACITY;
    return arr;
}

void clauseArrayDestroy(ClauseArray* arr) {
    if (arr) {
        if (arr->data) {
            // 清理每个子句的文字数组
            for (int i = 0; i < arr->size; i++) {
                destroyClause(&arr->data[i]);
            }
            free(arr->data);
        }
        free(arr);
    }
}

static void clauseArrayResize(ClauseArray* arr) {
    int newCapacity = arr->capacity * 2;
    Clause* newData = (Clause*)realloc(arr->data, newCapacity * sizeof(Clause));
    if (newData) {
        arr->data = newData;
        arr->capacity = newCapacity;
    }
}

void clauseArrayPush(ClauseArray* arr, Clause clause) {
    if (arr->size >= arr->capacity) {
        clauseArrayResize(arr);
    }
    arr->data[arr->size++] = clause;
}

Clause* clauseArrayGetPtr(ClauseArray* arr, int index) {
    if (index >= 0 && index < arr->size) {
        return &arr->data[index];
    }
    return NULL;
}