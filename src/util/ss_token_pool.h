#ifndef _SS_UTIL_SS_TOKEN_POOL_H_
#define _SS_UTIL_SS_TOKEN_POOL_H_


typedef struct ss_token_pool_s ss_token_pool_t;

struct ss_token_pool_s {
};

ss_token_pool_t *ss_token_pool_create();
void ss_token_pool_destroy(ss_token_pool_t *);
void ss_token_pool_initialize(ss_token_pool_t *);
void ss_token_pool_uninitialize(ss_token_pool_t *);

#endif // _SS_UTIL_SS_TOKEN_POOL_H_