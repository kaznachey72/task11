#include "hash_table.h"

#include <assert.h>


extern const int TOP_SIZE;

//--- pair_t<char*, ulong> -----------------------------------------------------

typedef long long pair_val_t;
typedef struct {
    gchar *key;
    pair_val_t val;
} pair_t;

gint pair_compare_by_val(gconstpointer lhs, gconstpointer rhs, gpointer userdata)
{   
    (void) userdata;
    return ((const pair_t *)rhs)->val - ((const pair_t *)lhs)->val;
}

void pair_print(gpointer kv, gpointer userdata)
{   
    (void) userdata;
    g_print("%13lld %s\n", ((const pair_t *)(kv))->val, ((const pair_t *)(kv))->key);
}

void on_pair_destroy(gpointer kv)
{
    if (kv) {
        g_free(((pair_t *)(kv))->key);
        g_free(kv);
    }
}

//--- Hash Table ---------------------------------------------------------------


void ht_insert(GHashTable *ht, gpointer key, gpointer value)
{
    gpointer ht_val = g_hash_table_lookup(ht, key);
    if (ht_val == NULL) {
        g_hash_table_insert(ht, g_strdup(key), value);
    }
    else {
        pair_val_t new_val = (pair_val_t)ht_val + (pair_val_t)value;
        g_hash_table_insert(ht, key, (void *)new_val);
    }
}

gboolean ht_merge(gpointer key, gpointer value, gpointer userdata)
{
    GHashTable *ht = (GHashTable *)(userdata);
    gpointer ht_val = g_hash_table_lookup(ht, key);
    if (ht_val == NULL) {
        g_hash_table_insert(ht, key, value);
    }
    else {
        pair_val_t new_val = (pair_val_t)ht_val + (pair_val_t)value;
        g_hash_table_insert(ht, key, (void *)new_val);
        g_free(key);
    }
    return true;
}


gboolean ht_iter_move(gpointer key, gpointer value, gpointer user_data)
{
    GSequence *sq = (GSequence *)(user_data);
    if (g_sequence_get_length(sq) < TOP_SIZE) {
        pair_t *kv = g_new(pair_t, 1);
        kv->key = key;
        kv->val = (pair_val_t)value;
        g_sequence_insert_sorted(sq, kv, pair_compare_by_val, NULL);
        return true;
    }
    pair_t *last_kv = (pair_t *)g_sequence_get(g_sequence_iter_prev(g_sequence_get_end_iter(sq)));
    assert(last_kv);
    if (last_kv->val >= (pair_val_t)value) {
        g_free(key);
        return true;
    }
    pair_t *kv = g_new(pair_t, 1);
    if (!kv){
        g_error("Ошибка выделения памяти");
        exit(EXIT_FAILURE);
    }
    kv->key = key;
    kv->val = (pair_val_t)value;
    g_sequence_insert_sorted(sq, kv, pair_compare_by_val, kv);
    g_sequence_remove(g_sequence_iter_prev(g_sequence_get_end_iter(sq)));
    return true;
}

