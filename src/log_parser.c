#include "log_parser.h"
#include "curl.h"

#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <gio/gio.h>


char *url_decode(const char *encoded_url)
{
    char *decoded_url = NULL;
    CURL *curl = curl_easy_init();
    if (curl) {
        int decodelen;
        char *decoded = curl_easy_unescape(curl, encoded_url, strlen(encoded_url), &decodelen);
        if (decoded) {
            decoded_url = strdup(decoded);
            curl_free(decoded);
        }
        curl_easy_cleanup(curl);
    }
    return decoded_url;
}

bool parse_log_line(char *str, char **url, char **refer, unsigned long *trafic)
{
    /*
        LogFormat "%v %l %u %t \"%r\" %>s %b"
        127.0.0.1 - frank [10/Oct/2000:13:55:36 -0700] "GET /apache_pb.gif HTTP/1.0" 200 2326
        "http://www.example.com/start.html" "Mozilla/4.08 [en] (Win98; I ;Nav)"
    */
    char *saveptr, *endptr, *str_trafic, *encoded_url;
    strtok_r(str, "\"", &saveptr);                // ......"|GET...
    strtok_r(NULL, " ", &saveptr);                // ...GET |url...
    encoded_url = strtok_r(NULL, " ", &saveptr);  // ...|url| HTTP/1.0"...
    strtok_r(NULL, "\"", &saveptr);               // ......"| 200...
    strtok_r(NULL, " ", &saveptr);                // ...200 |2326..
    str_trafic = strtok_r(NULL, "\"", &saveptr);  // ..|2326|...
    *refer = strtok_r(NULL, "\"", &saveptr);      // ...2326"|refer

    *url = url_decode(encoded_url);
    if (*url == NULL) {
        return false;
    }

    *trafic = strtol(str_trafic, &endptr, 10);
    if (endptr == str_trafic || errno == ERANGE) {
        return false;
    }

    if (*refer == NULL) {
        return false;
    }

    return true;
}

