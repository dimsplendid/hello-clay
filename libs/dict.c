#include <assert.h>
#include <stdlib.h>

// TODO: hash function
// int dict_hash(char *type, void *key);
// TODO: simillar API like tsoding's dynamic array

#define LOAD_FACTOR 0.65

typedef struct {
    // int hash;
    int key;
    int val;
} Dict_Item;

typedef struct {
    int *addr;
    Dict_Item *items;
    size_t count;
    size_t capacity;
} Dict;

typedef enum {
    NONE,
    SOME,
} Option_Tag;

typedef struct {
    Option_Tag option;
    int        val;
} Option_Int;

Option_Int dict_addr(Dict dict, int key) {
    
    // srand(0);
    for (
        int j = 0, i = (key % dict.capacity);
        j < dict.capacity;
        ++j,
        // i = (5 * i + 1 + rand()) % dict.capacity; // pseudo random probe, need modification
        i = (i + 1) % dict.capacity // linear probe
    ) {
        int addr = dict.addr[i];
        if (addr == -1)                  return (Option_Int){ NONE, i };
        if (dict.items[addr].key == key) return (Option_Int){ SOME, i };
    }
    return (Option_Int){ NONE, -1 };
}

void dict_alloc(Dict *dict, size_t capacity) {
    free(dict->addr);
    dict->addr = malloc(capacity * sizeof(int));
    assert(dict->addr != NULL && "Dict alloc fail: Buy more ram");
    dict->items = realloc(dict->items, capacity * sizeof(Dict_Item));
    assert(dict->items != NULL && "Dict alloc fail: Buy more ram");
    dict->capacity = capacity;
    for (size_t i = 0; i < capacity; ++i) {
        dict->addr[i] = -1; 
    }
    // re-calculate the key location
    for (size_t i = 0; i < dict->count; ++i) {
        Option_Int addr = dict_addr(*dict, dict->items[i].key);
        if ( addr.val >= 0 ) dict->addr[addr.val] = i;
    }
}

void dict_free(Dict *dict) {
    if (dict->capacity == 0) return;
    free(dict->items);
    free(dict->addr);
    dict->count = 0;
    dict->capacity = 0;
}


Option_Int dict_get(Dict dict, int key) {
    if (dict.count == 0) return (Option_Int) { .option = NONE };
    Option_Int i_addr = dict_addr(dict, key);
    switch (i_addr.option) {
        case NONE: return (Option_Int) { .option = NONE }; break;
        case SOME: return (Option_Int) { .option = SOME, .val = dict.items[dict.addr[i_addr.val]].val }; break;
    }
}

void dict_set(Dict *dict, int key, int val) {
    // init
    if (dict->count == 0) dict_alloc(dict, 8);
    
    Option_Int i_addr = dict_addr(*dict, key);

    switch (i_addr.option) {
        case SOME: 
            dict->items[dict->addr[i_addr.val]].val = val;
            break;
        case NONE:
            assert(i_addr.val >= 0 && "No Space for Dict");
            dict->addr[i_addr.val] = dict->count; 
            dict->items[dict->count++] = (Dict_Item) { key, val };
            break;
    }
 
    // resize
    if ((float)dict->count / (float)dict->capacity > LOAD_FACTOR) dict_alloc(dict, dict->capacity * 2);
}

// TODO
// void dict_del(Dict *dict, int key) {}

// #define DICT_DEBUG_MAIN
#ifdef DICT_DEBUG_MAIN

#include <stdio.h>
#include <math.h>

#define PROF
#include "./prof.c"

void dict_print(Dict dict) {
    printf("[count: %zu, capacity: %zu]\n", dict.count, dict.capacity);
    printf("{\n");
    for (size_t i = 0; i < dict.count; ++i) {
        printf("addr = %d\t    %d: %d\n", dict_addr(dict, dict.items[i].key).val, dict.items[i].key, dict.items[i].val);
        // printf("    %d: %d\n", dict.items[i].key, dict.items[i].val);
    }
    printf("}\n");
}

int fib(int n) {
    if (n <= 2) return 1;
    return fib(n-1) + fib(n-2);
}

Dict __fib_cache = {0};
int fib_cache(int n) {
    Option_Int result = dict_get(__fib_cache, n);
    switch (result.option) {
        case SOME: return result.val; break;
        case NONE:
            if (n<=2) {
                dict_set(&__fib_cache, n, 1);
                return 1;
            } else {
                dict_set(&__fib_cache, n, fib_cache(n-1) + fib_cache(n-2));
                return dict_get(__fib_cache, n).val;
            }
            break;
    }
}

int main(void) {

    begin_clock("Dict Random test"); {    
        Dict dict = {0};
        for (int i = 0; i < 19; ++i) {
            dict_set(&dict, i*i, i);
        }

        dict_set(&dict, 0, 69);
        dict_set(&dict, 69, 420);
    
        for (int i = 3; i < 10; ++i) {
            dict_set(&dict, (int)pow(2.0, (double)i), i);
        }

        dict_print(dict);

        printf("dict[0]:%s(%d)\n", dict_get(dict, 0).option ? "SOME" : "NONE" , dict_get(dict, 0).val);
        printf("dict[4]:%s(%d)\n", dict_get(dict, 4).option ? "SOME" : "NONE" , dict_get(dict, 4).val);
        printf("dict[9]:%s(%d)\n", dict_get(dict, 9).option ? "SOME" : "NONE" , dict_get(dict, 9).val);
        printf("dict[3]:%s(%d)\n", dict_get(dict, 3).option ? "SOME" : "NONE" , dict_get(dict, 3).val);
        printf("dict[69]:%s(%d)\n", dict_get(dict, 69).option ? "SOME" : "NONE" , dict_get(dict, 69).val);
        printf("dict[361]:%s(%d)\n", dict_get(dict, 361).option ? "SOME" : "NONE" , dict_get(dict, 361).val);
    
        dict_free(&dict);

    } end_clock();

    printf("------cache test------\n");

    begin_clock("Cache Test"); {
        begin_clock("Fib without cache"); {
            printf("fib(45) = %d\n", fib(45));
        } end_clock();

        begin_clock("Fib with cache"); {
            printf("fib_cache(45) = %d\n", fib_cache(45));
            dict_print(__fib_cache);
        } end_clock();
    } end_clock();

    printf("\n");
    
    dump_summary(stdout);
    
    return 0;
}
#endif // DICT_DEBUG_MAIN
