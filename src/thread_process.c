#include "thread_process.h"
#include "hash_table.h"
#include "log_parser.h"

#include <gio/gio.h>
#include <stdbool.h>


void *thread_process(void *data)
{
    thread_data_t *td = (thread_data_t *)(data);
    GError *err = NULL;

    GHashTable *ht_url = g_hash_table_new(g_str_hash, g_str_equal);     // url -> trafic
    GHashTable *ht_refer = g_hash_table_new(g_str_hash, g_str_equal);   // refer -> counts
    thread_result_t *res = g_new(thread_result_t, 1);
    if (!res || !ht_url || !ht_refer) {
        g_error("Ошибка выделения памяти");
        return NULL;
    }
    res->ht_url = ht_url;
    res->ht_refer = ht_refer;
    res->trafic = 0;

    gint i = 0;
    while ((uint)(i = g_atomic_int_add(&td->index, 1)) < td->files->len) {
        const gchar *file_path = g_ptr_array_index(td->files, i);
        g_message("разбор файла '%s'", file_path);
        GFile *f = g_file_new_for_path(file_path);
        if (!f) {
            g_error("Ошибка открытия файла: %s", file_path);
            continue;
        }
        GFileInputStream *fs = g_file_read(f, NULL, &err);
        if (!fs) {
            g_object_unref(f);
            g_error("Ошибка чтения файла: %s (%s)", file_path, err->message);
            g_error_free(err);
            continue;
        }
        GDataInputStream *ds = g_data_input_stream_new(G_INPUT_STREAM(fs));
        if (!ds) {
            g_error("Ошибка создания стрима для файла %s", file_path);
            g_object_unref(f);
            g_object_unref(fs);
            continue;
        }

        gchar *line;
        err = NULL;
        while ((line = g_data_input_stream_read_line(ds, NULL, NULL, &err)) != NULL && !err) {
            char *url = NULL, *refer = NULL;
            unsigned long trafic;
            if (parse_log_line(line, &url, &refer, &trafic) == true) {
                ht_insert(ht_url, url, (void *)trafic);
                ht_insert(ht_refer, refer, (void *)1);
                res->trafic += trafic;
            }
            g_free(line);
            g_free(url);
        }
        g_object_unref(ds);
        g_object_unref(f);
        if (err) {
            g_error("Ошибка чтения файла %s", file_path);
        }
    }
    return res;
}

