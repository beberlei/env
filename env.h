#ifndef ENV_H
#define ENV_H
void php_env_module_init(HashTable *vars TSRMLS_DC);
void php_env_request_init(HashTable *vars TSRMLS_DC);
#endif
