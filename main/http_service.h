#ifndef HTTP_SERVICE_H
#define HTTP_SERVICE_H
#include "app_context.h"

void http_service_start(app_context_t *ctx);
void http_service_stop(app_context_t *ctx);

#endif