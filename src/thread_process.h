#ifndef THREAD_PROCESS_H
#define THREAD_PROCESS_H

#include <glib.h>

typedef struct {
    GPtrArray *files;
    volatile gint index;
} thread_data_t;

typedef struct {
    GHashTable *ht_url;
    GHashTable *ht_refer;
    unsigned long trafic;
} thread_result_t;

void *thread_process(void *data);

#endif // THREAD_PROCESS_H
