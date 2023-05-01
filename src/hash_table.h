#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <glib.h>
#include <stdbool.h>

//--- pair_t<char*, long> ----------------------------------

void pair_print(gpointer kv, gpointer userdata);
void on_pair_destroy(gpointer kv);

//--- Hash Table -------------------------------------------

void ht_insert(GHashTable *ht, gpointer key, gpointer value);
gboolean ht_merge(gpointer key, gpointer value, gpointer userdata);
gboolean ht_iter_move(gpointer key, gpointer value, gpointer user_data);

#endif // HASH_TABLE_H
