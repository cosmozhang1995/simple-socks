#ifndef SOCKS5_CONFIG_H
#define SOCKS5_CONFIG_H

#include "common/ss_types.h"
#include "util/ss_strmap_def.h"

typedef struct {
    char username[256];
    char password[256];
} ss_auth_plain_entry_t;

typedef struct {
    ss_strmap_t map;
} ss_auth_plain_config_t;

typedef struct {
    ss_uint8_t auth_method;
    ss_auth_plain_config_t auth_plain;
} ss_config_t;

ss_config_t *ss_load_config(const char *path);
void ss_release_config(ss_config_t *config);

#endif
