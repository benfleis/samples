typedef struct http_io http_io_t;

http_io_t* http_io_create(void);
void http_io_destroy(http_io_t**);
