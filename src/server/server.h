typedef void(*client_receive_function_t)(int fd);
typedef void*(*create_client_context_function_t)();
typedef void*(*release_client_context_function_t)();

typedef struct {
    create_client_context_function_t client_context_create;
    release_client_context_function_t client_context_release;
    client_receive_function_t client_receive_handler;
} server_config_t;

int server(server_config_t config);