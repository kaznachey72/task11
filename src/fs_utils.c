#include "fs_utils.h"


GPtrArray *crate_all_files(const char *dirpath) 
{
    GError *error = NULL;
    GDir *dir = g_dir_open(dirpath, 0, &error);
    if (error) {
        g_error("%s", error->message);
        g_clear_error(&error);
        return NULL;
    }   
    GPtrArray *files = g_ptr_array_new();
    const gchar *filename;
    while ((filename = g_dir_read_name(dir))) {
        gchar *filepath = g_strdup_printf("%s/%s", dirpath, filename);
        if (g_file_test(filepath, G_FILE_TEST_IS_REGULAR)) {
            g_ptr_array_add(files, filepath);
        }   
        else {
            g_free(filepath);
        }   
    }   
    g_dir_close(dir);
    return files;
}

