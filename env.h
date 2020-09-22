#ifndef ENV_H
#define ENV_H

#ifndef TSRMLS_CC
#define TSRMLS_CC
#define TSRMLS_DC
#endif

void php_env_module_init(HashTable *vars TSRMLS_DC);
void php_env_request_init(HashTable *vars TSRMLS_DC);
#endif
