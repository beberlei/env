#ifndef ENV_H
#define ENV_H

int php_env_module_init(TSRMLS_D);
void php_env_module_shutdown();
void php_env_request_init();
#endif
