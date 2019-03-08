/**
 * @file hashtable.c
 * @brief Local hash-table implementation.
 */
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "hashtable.h"
#include "error.h"
#include "util.h"

struct bucket {
    kv_pair_t head;
    bucket_t* tail;
};

/**
 * @brief allocates memory for a new bucket and copies the pair into the
 *        hashtable
 * @param bucket pointer to the bucket where the pair should be allocated
 * @param key the key to copy
 * @param value the value to copy
 * @return any error_code if an error occurred
 */
error_code add_bucket(bucket_t* bucket, pps_key_t key, pps_value_t value)
{
    // Create the next bucket, acts as a NIL value for the linked list
    bucket->tail = malloc(sizeof(bucket_t));
    M_EXIT_IF_NULL(bucket->tail, (int) sizeof(bucket_t), "hashtable.c");

    // Create new pair for the current bucket
    memset(bucket->tail, 0, sizeof(bucket_t));

    // Copies the two strings into the pair
    bucket->head.key = strdup(key);
    bucket->head.value = strdup(value);

    if (bucket->head.key == NULL
        || bucket->head.value == NULL) {
        free(bucket->tail);
        bucket->tail = NULL;
        kv_pair_free(&bucket->head);
        bucket->head.key = NULL;
        bucket->head.value = NULL;
        return ERR_NOMEM;
    }
    debug_print("HASH-add: Key & value in hashtable: %s %d", bucket->head.key, bucket->head.value);
    return ERR_NONE;

}

error_code add_Htable_value(Htable_t table, pps_key_t key, pps_value_t value)
{
    M_EXIT_IF(table.buckets == NULL || key == NULL || value == NULL,
              ERR_BAD_PARAMETER, "hashtable.c", "%s", "HASH-add: NULL table or value given.");
    size_t idx = hash_function(key, table.size);
    bucket_t* bucket = &table.buckets[idx];
    while (bucket->head.key != NULL) {
        debug_print("%s", "HASH-add: collision found");
        if (strcmp(bucket->head.key, key) == 0) {
            pps_value_t old_value = bucket->head.value;
            bucket->head.value = strdup(value);
            if (bucket->head.value == NULL) {
                bucket->head.value = old_value;
                return ERR_NOMEM;
            }
            free_const_ptr(old_value);
            debug_print("%s", "HASH-add: value overwritten");
            return ERR_NONE;
        }
        debug_print("HASH-add: %s found, trying next key", bucket->head.key);
        bucket = bucket->tail;
    }

    return add_bucket(bucket, key, value);
}

pps_value_t get_Htable_value(Htable_t table, pps_key_t key)
{
    if (table.buckets != NULL && key != NULL) {
        size_t idx = hash_function(key, table.size);
        bucket_t* bucket = &table.buckets[idx];

        while (bucket != NULL && bucket->head.key != NULL) {

            if (strcmp(bucket->head.key, key) == 0) {
                debug_print("HASH-get: Request %s: found %s",
                            key, bucket->head.value);
                return strdup(bucket->head.value);
            }

            debug_print("HASH-get: Other key found: %s, trying next one",
                        bucket->head.key);
            bucket = bucket->tail;
        }
    }

    debug_print("HASH-get: no value found for the key %s", key);
    return NULL;
}

size_t hash_function(pps_key_t key, size_t size)
{
    M_REQUIRE(size != 0, __SIZE_MAX__, "size == %d", 0);
    M_REQUIRE_NON_NULL_CUSTOM_ERR(key, __SIZE_MAX__);

    size_t hash = 0;
    const size_t key_len = strlen(key);
    for (size_t i = 0; i < key_len; ++i) {
        hash += (unsigned char) key[i];
        hash += (hash << 10);
        hash ^= (hash >>  6);
    }
    hash += (hash <<  3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    debug_print("HASH-fun: index is %zu", hash % size);

    return hash % size;
}

Htable_t construct_Htable(size_t size)
{
    assert(size != 0);
    Htable_t table = {NULL, size};
    table.buckets = calloc(size, sizeof(bucket_t));
    if (table.buckets == NULL) {
        table.size = 0;
    }
    return table;
}

/**
 * @brief frees a linked list of bucket
 * @param bucket pointer to the first bucket of the linked list
 **/
void free_bucket(bucket_t* bucket)
{
    kv_pair_free(&bucket->head);
    bucket = bucket->tail;
    while (bucket != NULL) {
        kv_pair_free(&bucket->head);
        bucket_t* old_bucket = bucket;
        bucket = bucket->tail;
        free(old_bucket);
        old_bucket = NULL;
    }
}

void delete_Htable_and_content(Htable_t* table)
{
    if (table != NULL) {
        if (table->buckets != NULL) {
            for (size_t i = 0; i < table->size; ++i) {
                free_bucket(&table->buckets[i]);
            }
            free(table->buckets);
            table->buckets = NULL;
        }
        table->size = 0;
    }
}

void kv_pair_free(kv_pair_t* kv)
{
    if (kv != NULL) {
        free_const_ptr(kv->key);
        kv->key = NULL;
        free_const_ptr(kv->value);
        kv->value = NULL;
    }
}

kv_pair_t copy_pair(kv_pair_t kv)
{
    kv_pair_t new_kv = {strdup(kv.key), strdup(kv.value)};
    return new_kv;
}

kv_list_t* get_Htable_content(Htable_t table)
{
    kv_list_t* list = malloc(sizeof(kv_list_t));
    if (list == NULL) {
        return NULL;
    }
    memset(list, 0, sizeof(kv_list_t));
    for (size_t i = 0; i < table.size; ++i) {
        bucket_t* bucket = &table.buckets[i];
        while (bucket->head.key != NULL) {
            kv_pair_t* old_pairs = list->pairs;
            list->pairs = realloc(list->pairs, (list->size + 1) * sizeof(kv_pair_t));
            if (list->pairs == NULL) {
                list->pairs = old_pairs;
                kv_list_free(list);
                //correcteur: kv_list_free appel déjà free sur list (-0.5)
                free(list);
                return NULL;
            }
            list->pairs[list->size] = copy_pair(bucket->head);
            bucket = bucket->tail;
            list->size += 1;
        }
    }

    return list;
}

void kv_list_free(kv_list_t* list)
{
    if (list != NULL) {
        if (list->pairs != NULL) {
            for (size_t i = 0; i < list->size; ++i) {
                kv_pair_free(&list->pairs[i]);
            }
            free(list->pairs);
            list->pairs = NULL;
        }
        list->size = 0;
    }
    free(list);
}
