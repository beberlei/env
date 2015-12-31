#include "php.h"
#include "../php_env.h"
#include "../env.h"

ZEND_DECLARE_MODULE_GLOBALS(env)

static void php_env_ini_parser_cb(zval *key, zval *value, zval *index, int callback_type, HashTable *arg) /* {{{ */ {
	zval *rv;

	TSRMLS_FETCH();

	if (ENV_G(parse_err)) {
		return;
	}

	if (value == NULL) {
		return;
	}

	if (callback_type == ZEND_INI_PARSER_ENTRY && Z_TYPE_P(value) == IS_STRING) {
		setenv(Z_STRVAL_P(key), Z_STRVAL_P(value), 1);
	} else if (callback_type == ZEND_INI_PARSER_SECTION || callback_type == ZEND_INI_PARSER_POP_ENTRY) {
		ENV_G(parse_err) = 1;
	}
}

int php_env_module_init(TSRMLS_D) {
	int ndir = 255;
	unsigned char c;
	zend_file_handle fh = {0};
	HashTable *env_container;

	if (ENV_G(file) != NULL && strlen(ENV_G(file)) > 0 && VCWD_ACCESS(ENV_G(file), F_OK) == 0) {
		if ((fh.handle.fp = VCWD_FOPEN(ENV_G(file), "r"))) {
			fh.filename = ENV_G(file);
			fh.type = ZEND_HANDLE_FP;

			ALLOC_HASHTABLE(env_container);
			zend_hash_init(env_container, 255, NULL, NULL, 1);

			if (zend_parse_ini_file(&fh, 0, 0, (zend_ini_parser_cb_t)php_env_ini_parser_cb, env_container TSRMLS_CC) == FAILURE || ENV_G(parse_err)) {
				if (ENV_G(parse_err)) {
					php_error(E_WARNING, "env: parsing '%s' failed", ENV_G(file));
				}
				ENV_G(parse_err) = 0;
				efree(env_container);
				return SUCCESS;
			}
			efree(env_container);
		}
	}

	return SUCCESS;
}
