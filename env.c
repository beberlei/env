/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_env.h"

ZEND_DECLARE_MODULE_GLOBALS(env)

/* True global resources - no need for thread safety here */
static int le_env;
static HashTable *env_container;

static void php_env_zval_persistent(zval *zv, zval *rv);

#define PALLOC_HASHTABLE(ht)   do {                         \
	(ht) = (HashTable*)pemalloc(sizeof(HashTable), 1);    \
	if ((ht) == NULL) {                                     \
		zend_error(E_ERROR, "Cannot allocate HashTable, not enough memory?");  \
	}                                                       \
} while(0)

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("env.file", "/etc/php/.env", PHP_INI_ALL, OnUpdateString, file, zend_env_globals, env_globals)
PHP_INI_END()
/* }}} */


/* {{{ php_env_init_globals
 */
static void php_env_init_globals(zend_env_globals *env_globals)
{
	env_globals->file = NULL;
}

static void php_env_hash_init(zval *zv, size_t size) /* {{{ */ {
	HashTable *ht;
	PALLOC_HASHTABLE(ht);
	zend_hash_init(ht, size, NULL, NULL, 1);
	GC_FLAGS(ht) |= IS_ARRAY_IMMUTABLE;
	ZVAL_ARR(zv, ht);
	Z_ADDREF_P(zv);
	Z_TYPE_FLAGS_P(zv) = IS_TYPE_IMMUTABLE;
} 

static void php_env_hash_destroy(HashTable *ht) /* {{{ */ {
	zend_string *key;
	zval *element;

	if (((ht)->u.flags & HASH_FLAG_INITIALIZED)) {
		ZEND_HASH_FOREACH_STR_KEY_VAL(ht, key, element) {
			if (key) {
				free(key);
			}
			switch (Z_TYPE_P(element)) {
				case IS_PTR:
				case IS_STRING:
					free(Z_PTR_P(element));
					break;
				case IS_ARRAY:
					php_env_hash_destroy(Z_ARRVAL_P(element));
					break;
			}
		} ZEND_HASH_FOREACH_END();
		free(HT_GET_DATA_ADDR(ht));
	}
	free(ht);
} /* }}} */

static zval* php_env_symtable_update(HashTable *ht, zend_string *key, zval *zv) /* {{{ */ {
	zend_ulong idx;
	if (ZEND_HANDLE_NUMERIC(key, idx)) {
		free(key);
		return zend_hash_index_update(ht, idx, zv);
	} else {
		return zend_hash_update(ht, key, zv);
	}
}
/* }}} */

static zend_string* php_env_str_persistent(char *str, size_t len) /* {{{ */ {
	zend_string *key = zend_string_init(str, len, 1);
	if (key == NULL) {
		zend_error(E_ERROR, "Cannot allocate string, not enough memory?");
	}
	key->h = zend_string_hash_val(key);
	GC_FLAGS(key) |= IS_STR_INTERNED | IS_STR_PERMANENT;
	return key;
}
/* }}} */
	
static void php_env_hash_copy(HashTable *target, HashTable *source) /* {{{ */ {
	zend_string *key;
	zend_long idx;
	zval *element, rv;

	ZEND_HASH_FOREACH_KEY_VAL(source, idx, key, element) {
		php_env_zval_persistent(element, &rv);
		if (key) {
			zend_hash_update(target, php_env_str_persistent(ZSTR_VAL(key), ZSTR_LEN(key)), &rv);
		} else {
			zend_hash_index_update(target, idx, &rv);
		}
	} ZEND_HASH_FOREACH_END();
} /* }}} */

static void php_env_zval_persistent(zval *zv, zval *rv) /* {{{ */ {
	switch (Z_TYPE_P(zv)) {
		case IS_CONSTANT:
		case IS_STRING:
			ZVAL_INTERNED_STR(rv, php_env_str_persistent(Z_STRVAL_P(zv), Z_STRLEN_P(zv)));
			break;
		case IS_ARRAY:
			{
				php_env_hash_init(rv, zend_hash_num_elements(Z_ARRVAL_P(zv)));
				php_env_hash_copy(Z_ARRVAL_P(rv), Z_ARRVAL_P(zv));
			}
			break;
		case IS_RESOURCE:
		case IS_OBJECT:
		case _IS_BOOL:
		case IS_LONG:
		case IS_NULL:
			ZEND_ASSERT(0);
			break;
	}
} /* }}} */

zend_string *php_env_concat3(char *str1, size_t str1_len, char *str2, size_t str2_len, char *str3, size_t str3_len) /* {{{ */
{
	size_t len = str1_len + str2_len + str3_len;
	zend_string *res = zend_string_alloc(len, 0);

	memcpy(ZSTR_VAL(res), str1, str1_len);
	memcpy(ZSTR_VAL(res) + str1_len, str2, str2_len);
	memcpy(ZSTR_VAL(res) + str1_len + str2_len, str3, str3_len);
	ZSTR_VAL(res)[len] = '\0';

	return res;
}

zend_string *php_env_concat_env(char *name1, size_t name1_len, char *name2, size_t name2_len) {
	return php_env_concat3(name1, name1_len, "=", 1, name2, name2_len);
}    

static void php_env_ini_parser_cb(zval *key, zval *value, zval *index, int callback_type, void *arg) /* {{{ */ {
	zval *arr = (zval *)arg;
	zend_string *var;

	if (ENV_G(parse_err)) {
		return;
	}

	char *seg, *skey, *ptr;
	zval *pzval, rv;

	if (value == NULL) {
		return;
	}

	if (callback_type == ZEND_INI_PARSER_ENTRY) {
		php_env_zval_persistent(value, &rv);
		php_env_symtable_update(Z_ARRVAL_P(arr), php_env_str_persistent(Z_STRVAL_P(key), Z_STRLEN_P(key)), &rv);
	}
}

/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(env)
{
	REGISTER_INI_ENTRIES();

	zval result;
	int ndir = 255;
	uint32_t i;
	unsigned char c;
	struct zend_stat sb;
	zend_file_handle fh = {0};

	if (ENV_G(file) != NULL && VCWD_STAT(ENV_G(file), &sb) == 0) {
		if (S_ISREG(sb.st_mode)) {
			if ((fh.handle.fp = VCWD_FOPEN(ENV_G(file), "r"))) {
				fh.filename = ENV_G(file);
				fh.type = ZEND_HANDLE_FP;
				php_env_hash_init(&result, 128);

				PALLOC_HASHTABLE(env_container);
				zend_hash_init(env_container, 255, NULL, NULL, 1);

				if (zend_parse_ini_file(&fh, 0, 0 /* ZEND_INI_SCANNER_NORMAL */,
							php_env_ini_parser_cb, (void *)&result) == FAILURE || ENV_G(parse_err)) {
					if (!ENV_G(parse_err)) {
						php_error(E_WARNING, "Parsing '%s' failed", ENV_G(file));
					}
					ENV_G(parse_err) = 0;
					php_env_hash_destroy(Z_ARRVAL(result));
					return FAILURE;
				}

				env_container = Z_ARRVAL(result);
			}
		}
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(env)
{
	UNREGISTER_INI_ENTRIES();

	if (env_container) {
		php_env_hash_destroy(env_container);
	}

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(env)
{
	zend_string *key, *var;
	zend_long idx;
	zval *element, rv;

	if (env_container) {
		ZEND_HASH_FOREACH_KEY_VAL(env_container, idx, key, element) {
			if (key) {
				var = php_env_concat_env(ZSTR_VAL(key), ZSTR_LEN(key), Z_STRVAL_P(element), Z_STRLEN_P(element));
				putenv(ZSTR_VAL(var));
				//zend_string_release(var);
			}
		} ZEND_HASH_FOREACH_END();
	}

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(env)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(env)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "env support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ env_functions[]
 *
 * Every user visible function must have an entry in env_functions[].
 */
const zend_function_entry env_functions[] = {
	PHP_FE_END	/* Must be the last line in env_functions[] */
};
/* }}} */

/* {{{ env_module_entry
 */
zend_module_entry env_module_entry = {
	STANDARD_MODULE_HEADER,
	"env",
	env_functions,
	PHP_MINIT(env),
	PHP_MSHUTDOWN(env),
	PHP_RINIT(env),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(env),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(env),
	PHP_ENV_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_ENV
ZEND_GET_MODULE(env)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
