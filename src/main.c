#include <ctype.h>
#include <errno.h>
#include <gio/gio.h>
#include <locale.h>

#include "curl.h"
#include "fs_utils.h"
#include "hash_table.h"
#include "thread_process.h"
#include "log_parser.h"


const int TOP_SIZE = 10;
const char *dirlogs = "../../logs";
int nthreads = 4;

void usage(char *prog_name) 
{
    printf("Использование: %s [ПАРАМЕТР]...\n"
           "ПАРАМЕТРЫ:\n"
           "   -d <dir> - директория логов (по умолчанию '%s')\n"
           "   -n <num> - количество потоков (по умолчанию %d)\n"
           "   -h       - справка\n",
           prog_name, dirlogs, nthreads);
}

int main(int argc, char *argv[]) 
{
    setlocale(LC_ALL, "ru_RU.UTF-8");

    {
        int c;
        while ((c = getopt(argc, argv, "d:n:h")) != -1) {
            switch (c) {
                case 'd': 
                    dirlogs = optarg;
                    break;
                case 'n': {
                    char *endptr;
                    nthreads = strtol(optarg, &endptr, 10);
                    if (endptr == optarg || errno == ERANGE || nthreads <= 0) {
                        usage(argv[0]);
                        return EXIT_FAILURE;
                    }
                    break; 
                }
                case 'h':
                    usage(argv[0]);
                    return EXIT_SUCCESS;
                default:
                    usage(argv[0]);
                    return EXIT_FAILURE;
            }
        }
    }

    g_print("Разбор директории '%s' запущен в %d поток(а/ов)\n", dirlogs, nthreads);

    GPtrArray *files = crate_all_files(dirlogs);
    if (!files) {
        return EXIT_FAILURE;
    }
    if (files->len == 0) {
        g_print("Директория с логами путста\n");
        g_ptr_array_free(files, TRUE);
        return EXIT_SUCCESS;
    }

    GSequence *sq_top_urls = g_sequence_new(on_pair_destroy);
    GSequence *sq_top_refers = g_sequence_new(on_pair_destroy);
    GHashTable *ht_sum_url = NULL;
    GHashTable *ht_sum_refer = NULL;
    unsigned long sum_trafic = 0;

    GPtrArray *threads = g_ptr_array_new();
    if (!sq_top_urls || !sq_top_refers || !threads) {
        g_error("Ошибка выделения памяти");
        return EXIT_FAILURE;
    }

    thread_data_t td = {files, 0u};

    for (guint i = 0; i < MIN(files->len, (guint)(nthreads)); ++i) {
        GThread *t = g_thread_new("thread", thread_process, &td);
        if (!t) {
            g_error("Ошибка создания потока");
            exit(EXIT_FAILURE);
        }
        g_ptr_array_add(threads, t);
    }
    thread_result_t *res = NULL;
    for (guint i = 0; i < threads->len; i++) {
        GThread *t = (GThread *)g_ptr_array_index(threads, i);
        res = (thread_result_t *)g_thread_join(t);
        if (!res) {
            break;
        }
        sum_trafic += res->trafic;
        if (ht_sum_refer == NULL || ht_sum_url == NULL) {
            ht_sum_refer = res->ht_refer;
            ht_sum_url = res->ht_url;
        } 
        else {
            g_hash_table_foreach_remove(res->ht_url, ht_merge, ht_sum_url);
            g_hash_table_destroy(res->ht_url);
            g_hash_table_foreach_remove(res->ht_refer, ht_merge, ht_sum_refer);
            g_hash_table_destroy(res->ht_refer);
        }
        g_free(res);
    }
    if (res) {
        g_hash_table_foreach_remove(ht_sum_url, ht_iter_move, sq_top_urls);
        g_hash_table_foreach_remove(ht_sum_refer, ht_iter_move, sq_top_refers);
        g_print("\n\nОбщее количество отданных байт: %lu\n", sum_trafic);

        g_print("\n%d самых \"тяжёлых\" по суммарному трафику URL'ов:\n", TOP_SIZE);
        g_sequence_foreach(sq_top_urls, pair_print, NULL);

        g_print("\n%d наиболее часто встречающихся Referer’ов:\n", TOP_SIZE);
        g_sequence_foreach(sq_top_refers, pair_print, NULL);
    }

    if (ht_sum_url) {
        g_hash_table_destroy(ht_sum_url);
    }
    g_sequence_free(sq_top_urls);
    if (ht_sum_refer) {
        g_hash_table_destroy(ht_sum_refer);
    }
    g_sequence_free(sq_top_refers);
    g_ptr_array_set_free_func(files, g_free);
    g_ptr_array_free(files, TRUE);
    g_ptr_array_free(threads, TRUE);
    
    return (res != NULL) ? EXIT_SUCCESS: EXIT_FAILURE;
}
