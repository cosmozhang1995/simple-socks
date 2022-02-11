typedef void*(*create_client_context_function_t)();
typedef void*(*release_client_context_function_t)();
typedef void(*client_recv_function_t)(int fd, void *context);
typedef void(*client_send_function_t)(int fd, void *context);

typedef struct {
    create_client_context_function_t create_client_context_function;
    release_client_context_function_t release_client_context_function;
    client_recv_function_t client_recv_handler;
    client_send_function_t client_send_handler;
} server_config_t;

int server(server_config_t config);