#include "jsonx.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

enum json_errno {
    JSON_ERR_OK = 0,
    JSON_ERR_BRACE,
    JSON_ERR_KEY,
    JSON_ERR_TYPE,
};

struct json_object {
    char *raw;
    int errno;
};

struct json_object *json_object_new(const char *str)
{
    struct json_object *jo = malloc(sizeof(*jo));
    memset(jo, 0, sizeof(*jo));
    jo->raw = strdup(str);
    return jo;
}

void json_object_delete(struct json_object *jo)
{
    assert(jo);
    free(jo->raw);
    free(jo);
}

static int
__json_get_int(struct json_object *jo, const char *str, const char *path)
{
    assert(path[0] == '/');
    assert(path[1] != 0);

    char *path_next = strchr(path + 1, '/');

    // recursive fininshed
    if (path_next == NULL) {
        const char *key = path + 1;
        int find_key = 0;
        int brace_cnt = 0;

        for (int i = 0; i < strlen(str); i++) {
            if (find_key == 0) {
                if (str[i] == '{') {
                    brace_cnt++;
                    continue;
                }
                if (str[i] == '}') {
                    brace_cnt--;
                    continue;
                }
                if (brace_cnt == 1) {
                    if (key[0] == str[i] &&
                        memcmp(key, str + i, strlen(key)) == 0) {
                        find_key = 1;
                        i += strlen(key) - 1;
                        continue;
                    }
                }
            } else {
                if (str[i] == ' ' || str[i] == ':')
                    continue;
                if (!isdigit(str[i])) {
                    jo->errno = JSON_ERR_TYPE;
                    return -1;
                }
                char *value_end = NULL;
                int j = i;
                for (; j < strlen(str); j++) {
                    if (value_end == NULL && isdigit(str[j]))
                        continue;
                    if (str[j] == ' ') {
                        if (value_end == NULL)
                            value_end = (char *)(str + j);
                        continue;
                    }
                    if (str[j] == ',' || str[j] == '}') {
                        if (value_end == NULL)
                            value_end = (char *)(str + j);
                        break;
                    } else {
                        jo->errno = JSON_ERR_TYPE;
                        return -1;
                    }
                }
                if (j == strlen(str)) {
                    jo->errno = JSON_ERR_BRACE;
                    return -1;
                }
                assert(value_end != NULL);
                char tmp[16] = {0};
                memcpy(tmp, str + i, value_end - str - i);
                return atoi(tmp);
            }
        }
        if (find_key == 0)
            jo->errno = JSON_ERR_KEY;
        else
            jo->errno = JSON_ERR_TYPE;
        return -1;
    } else {
        char *key = malloc(path_next - path);
        memset(key, 0, path_next - path);
        memcpy(key, path + 1, path_next - path - 1);
        int find_key = 0;
        int brace_cnt = 0;

        for (int i = 0; i < strlen(str); i++) {
            if (find_key == 0) {
                if (str[i] == '{') {
                    brace_cnt++;
                    continue;
                }
                if (str[i] == '}') {
                    brace_cnt--;
                    continue;
                }
                if (brace_cnt == 1) {
                    if (key[0] == str[i] &&
                        memcmp(key, str + i, strlen(key)) == 0) {
                        find_key = 1;
                        i += strlen(key) - 1;
                        continue;
                    }
                }
            } else {
                if (str[i] == ' ' || str[i] == ':')
                    continue;
                if (str[i] != '{') {
                    jo->errno = JSON_ERR_TYPE;
                    free(key);
                    return -1;
                }
                free(key);
                return __json_get_int(jo, str + i, path_next);
            }
        }

        free(key);
        if (find_key == 0)
            jo->errno = JSON_ERR_KEY;
        else
            jo->errno = JSON_ERR_TYPE;
        return -1;
    }
}

int json_get_int(struct json_object *jo, const char *path)
{
    return __json_get_int(jo, jo->raw, path);
}