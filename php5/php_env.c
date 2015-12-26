#include "php.h"
#include "../php_env.h"
#include "../env.h"

ZEND_DECLARE_MODULE_GLOBALS(env)

static HashTable *env_container;

int php_env_module_init() {
}
void php_env_request_init() {
}
void php_env_module_shutdown() {
}
