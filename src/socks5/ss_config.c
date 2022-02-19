#include "socks5/ss_config.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "socks5/ss_auth_method.h"
#include "util/ss_strmap.h"
#include "util/ss_string_utils.h"

static ss_config_t *ss_create_config();
static ss_int8_t read_config_line(int fd, char *key, char *value);
static ss_int8_t parse_auth_method(const char *method_str, ss_uint8_t *result);
static ss_int8_t parse_auth_plain(const char *filepath, ss_auth_plain_config_t *result);
static void auth_plain_initialize(ss_auth_plain_config_t *config);
static void auth_plain_uninitialize(ss_auth_plain_config_t *config);
static void release_auth_plain(ss_auth_plain_config_t *result);

ss_config_t *ss_load_config(const char *path)
{
    ss_config_t *config;
    char rd_key[1024], rd_value[1024];
    int retcode, fd;

    fd = open(path, O_RDONLY);
    if (fd < 0) return 0;

    config = ss_create_config();

    while (read_config_line(fd, rd_key, rd_value) == 0) {
        if (rd_key[0] == '\0') continue;
        if (strcmp(rd_key, "auth") == 0) {
            retcode = parse_auth_method(rd_value, &config->auth_method);
            if (retcode != 0) {
                printf("invalid auth method.\n");
                goto _l_error;
            }
        }
        else if (strcmp(rd_key, "auth_plain") == 0) {
            retcode = parse_auth_plain(rd_value, &config->auth_plain);
            if (retcode != 0) {
                printf("invalid auth plain.\n");
                goto _l_error;
            }
        }
    }
    goto _l_end;

_l_error:
    ss_release_config(config);
    config = 0;

_l_end:
    return config;
}

static ss_config_t *ss_create_config()
{
    ss_config_t *config;
    config = malloc(sizeof(ss_config_t));
    config->auth_method = SOCKS5_AUTH_NONE;
    auth_plain_initialize(&config->auth_plain);
}

void ss_release_config(ss_config_t *config)
{
    auth_plain_uninitialize(&config->auth_plain);
    free(config);
}

static ss_int8_t read_config_line(int fd, char *key, char *value)
{
    char ch;
    ssize_t rc;
    size_t key_length = 0, value_length = 0;

_l_loop_read_key:
    rc = read(fd, &ch, 1);
    if (rc <= 0) goto _l_end;
    switch (ch) {
    case '=':  goto _l_loop_read_value;
    case '\n': goto _l_end;
    }
    key[key_length++] = ch;
    goto _l_loop_read_key;

_l_loop_read_value:
    rc = read(fd, &ch, 1);
    if (rc <= 0) goto _l_end;
    if (ch == '\n') goto _l_end;
    value[value_length++] = ch;
    goto _l_loop_read_value;

_l_end:
    key[key_length] = '\0';
    value[value_length] = '\0';
    ss_string_trim(key);
    ss_string_trim(value);
    return rc > 0 ? 0 : -1;
}

static ss_int8_t parse_auth_method(const char *method_str, ss_uint8_t *result)
{
    if (strcmp(method_str, "none") == 0) {
        *result = SOCKS5_AUTH_NONE;
        return 0;
    }
    if (strcmp(method_str, "plain") == 0) {
        *result = SOCKS5_AUTH_PLAIN;
        return 0;
    }
    return -1;
}

static ss_int8_t read_auth_plain_line(int fd,
    char *username, size_t max_ulen,
    char *password, size_t max_plen)
{
    char ch;
    ssize_t rc;
    size_t ulen = 0, plen = 0;

_l_loop_read_username:
    rc = read(fd, &ch, 1);
    if (rc <= 0)
        goto _l_end;
    switch (ch) {
    case ' ':
    case '\t':
        if (ulen > 0)
            goto _l_loop_read_password;
        break;
    case '\n':
    case '\r':
        goto _l_end;
    }
    username[ulen++] = ch;
    if (ulen > max_ulen)
        goto _l_error;
    goto _l_loop_read_username;

_l_loop_read_password:
    rc = read(fd, &ch, 1);
    if (rc <= 0)
        goto _l_end;
    switch (ch) {
    case ' ':
    case '\t':
    case '\n':
    case '\r':
        goto _l_end;
    }
    password[plen++] = ch;
    if (plen > max_plen)
        goto _l_error;
    goto _l_loop_read_password;

_l_end:
    username[ulen] = '\0';
    password[plen] = '\0';
    return rc == 0 ? 0 : 1;

_l_error:
    username[0] = '\0';
    password[0] = '\0';
    return -1;
}

static ss_bool_t strmap_release_string_value(const char *key, ss_variable_t *value, void *arg) {
    if (value->ptr) {
        free(value->ptr);
        value->ptr = SS_NULL;
    }
    return SS_TRUE;
}

static ss_int8_t parse_auth_plain(const char *filepath, ss_auth_plain_config_t *config)
{
    int fd;
    ss_int8_t rc;
    char username[256], password[256];
    ss_variable_t mapvalue, temp;
    fd = open(filepath, O_RDONLY);
    if (fd < 0) return -1;
    while ((rc == read_auth_plain_line(fd,
        username, sizeof(username) - 1,
        password, sizeof(password) - 1)) == 0)
    {
        if (ss_string_empty(username) || ss_string_empty(password)) continue;
        mapvalue.ptr = ss_string_clone(password);
        if (ss_strmap_put(&config->map, username, mapvalue, &temp))
            strmap_release_string_value(SS_NULL, &temp, SS_NULL);
    }
    if (rc < 0) return -1;
    return 0;
}

static void auth_plain_initialize(ss_auth_plain_config_t *config)
{
    ss_strmap_initialize(&config->map);
}

static void auth_plain_uninitialize(ss_auth_plain_config_t *config)
{
    ss_strmap_foreach(&config->map, strmap_release_string_value, SS_NULL);
    ss_strmap_uninitialize(&config->map);
}
