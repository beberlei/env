#include "php.h"
#include "../php_env.h"
#include "../env.h"

ZEND_DECLARE_MODULE_GLOBALS(env)

static HashTable *env_container;

#define PALLOC_HASHTABLE(ht)   do {                         \
(ht) = (HashTable*)pemalloc(sizeof(HashTable), 1);    \
if ((ht) == NULL) {                                     \
	zend_error(E_ERROR, "Cannot allocate HashTable, not enough memory?");  \
}                                                       \
} while(0)

static void php_env_hash_init(zval *zv, size_t size) /* {{{ */ {
	HashTable *ht;
	PALLOC_HASHTABLE(ht);
	zend_hash_init(ht, size, NULL, NULL, 1);
	Z_ARRVAL_P(zv) = ht;
	Z_ADDREF_P(zv);
}

static void php_env_hash_destroy(HashTable *ht) /* {{{ */ {
}

static char* php_env_str_persistent(char *str, size_t len) /* {{{ */ {
	char *key;
	key = (char*)pemalloc(len+1, 1);
	memcpy(key, str, len);
	key[len] = '\0';
	return key;
}

static zval* php_env_zval_persistent(zval *zv) /* {{{ */ {
	zval *rv;

	switch (Z_TYPE_P(zv)) {
		case IS_CONSTANT:
		case IS_STRING:
			rv = (zval*)pemalloc(sizeof(zval), 1);
			if (rv == NULL) {                                     \
				zend_error(E_ERROR, "Cannot allocate zval, not enough memory?");  \
			}
			INIT_PZVAL(rv);
			Z_TYPE_P(rv) = Z_TYPE_P(zv);
			Z_STRVAL_P(rv) = pestrdup(Z_STRVAL_P(zv), Z_STRLEN_P(zv));
			Z_STRLEN_P(rv) = Z_STRLEN_P(zv);
			break;
		case IS_ARRAY:
		case IS_RESOURCE:
		case IS_OBJECT:
		case IS_LONG:
		case IS_NULL:
			ZEND_ASSERT(0);
			break;
	}

	return rv;
} /* }}} */

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
		rv = php_env_zval_persistent(value);
		zend_hash_update(arg, php_env_str_persistent(Z_STRVAL_P(key), Z_STRLEN_P(key)), Z_STRLEN_P(key), &rv, sizeof(zval*), NULL);
	} else {
		ENV_G(parse_err) = 1;
	}
}

int php_env_module_init(TSRMLS_D) {
	int ndir = 255;
	unsigned char c;
	zend_file_handle fh = {0};

	if (ENV_G(file) != NULL && strlen(ENV_G(file)) > 0 && VCWD_ACCESS(ENV_G(file), F_OK) == 0) {
		if ((fh.handle.fp = VCWD_FOPEN(ENV_G(file), "r"))) {
			fh.filename = ENV_G(file);
			fh.type = ZEND_HANDLE_FP;

			PALLOC_HASHTABLE(env_container);
			zend_hash_init(env_container, 255, NULL, NULL, 1);

			if (zend_parse_ini_file(&fh, 0, 0, (zend_ini_parser_cb_t)php_env_ini_parser_cb, env_container TSRMLS_CC) == FAILURE || ENV_G(parse_err)) {
				if (ENV_G(parse_err)) {
					php_error(E_WARNING, "env: parsing '%s' failed", ENV_G(file));
				}
				ENV_G(parse_err) = 0;
				php_env_hash_destroy(env_container);
				return SUCCESS;
			}
		}
	}

	return SUCCESS;
}

char *php_env_concat3(char *str1, size_t str1_len, char *str2, size_t str2_len, char *str3, size_t str3_len) /* {{{ */
{
	size_t len = str1_len + str2_len + str3_len;
	char *res = emalloc(len);

	memcpy(res, str1, str1_len);
	memcpy(res + str1_len, str2, str2_len);
	memcpy(res + str1_len + str2_len, str3, str3_len);
	res[len] = '\0';

	return res;
}

char *php_env_concat_env(char *name1, size_t name1_len, char *name2, size_t name2_len) {
	return php_env_concat3(name1, name1_len, "=", 1, name2, name2_len);
}

void php_env_request_init() {
	char *key, *var;
	uint len;
	ulong idx;
	int type;
	zval **element;

	if (!env_container) {
		return;
	}

	for (zend_hash_internal_pointer_reset(env_container);
			zend_hash_has_more_elements(env_container) == SUCCESS;
			zend_hash_move_forward(env_container)) {

		type = zend_hash_get_current_key_ex(env_container, &key, &len, &idx, 0, NULL);

		if (zend_hash_get_current_data(env_container, (void**)&element) == SUCCESS && element != NULL && Z_TYPE_PP(element) == IS_STRING) {
			var = php_env_concat_env(key, len, Z_STRVAL_PP(element), Z_STRLEN_PP(element));
			putenv(var);
		}
	}
}
void php_env_module_shutdown() {
	if (env_container) {
		php_env_hash_destroy(env_container);
	}
}
