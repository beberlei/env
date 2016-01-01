#include "php.h"
#include "../php_env.h"
#include "../env.h"

ZEND_DECLARE_MODULE_GLOBALS(env)

static void php_env_init_globals(zend_env_globals *env_globals)
{
	env_globals->file = NULL;
	env_globals->parse_err = 0;
	env_globals->vars = (HashTable*)pemalloc(sizeof(HashTable), 1);
	zend_hash_init(env_globals->vars, 128, NULL, char_ptr_dtor, 1);
}

static void php_env_ini_parser_cb(zval *key, zval *value, zval *index, int callback_type, HashTable *arg) /* {{{ */ {
	zval *rv;
	char *str;

	TSRMLS_FETCH();

	if (ENV_G(parse_err)) {
		return;
	}

	if (value == NULL) {
		return;
	}

	if (callback_type == ZEND_INI_PARSER_ENTRY && Z_TYPE_P(value) == IS_STRING) {
		str = strndup(Z_STRVAL_P(value), Z_STRLEN_P(value));
		zend_symtable_update(arg, Z_STRVAL_P(key), Z_STRLEN_P(key), &str, sizeof(char*), NULL);
	} else if (callback_type == ZEND_INI_PARSER_SECTION || callback_type == ZEND_INI_PARSER_POP_ENTRY) {
		ENV_G(parse_err) = 1;
	}
}

void php_env_module_init(HashTable *vars TSRMLS_DC) {
	int ndir = 255;
	unsigned char c;
	zend_file_handle fh = {0};

	if (ENV_G(file) != NULL && strlen(ENV_G(file)) > 0 && VCWD_ACCESS(ENV_G(file), F_OK) == 0) {
		if ((fh.handle.fp = VCWD_FOPEN(ENV_G(file), "r"))) {
			fh.filename = ENV_G(file);
			fh.type = ZEND_HANDLE_FP;

			if (zend_parse_ini_file(&fh, 0, 0, (zend_ini_parser_cb_t)php_env_ini_parser_cb, vars TSRMLS_CC) == FAILURE || ENV_G(parse_err)) {
				if (ENV_G(parse_err)) {
					php_error(E_WARNING, "env: parsing '%s' failed", ENV_G(file));
				}
				ENV_G(parse_err) = 0;
			}
		}
	}
}

void php_env_request_init(HashTable *vars TSRMLS_DC)
{
	char  *str;
	uint   len;
	ulong  idx;
	int    type;
	char **data;

	for (zend_hash_internal_pointer_reset(vars);
			zend_hash_has_more_elements(vars) == SUCCESS;
			zend_hash_move_forward(vars)) {

		type = zend_hash_get_current_key_ex(vars, &str, &len, &idx, 0, NULL);
		if (type == HASH_KEY_IS_STRING) {
			if ((zend_hash_get_current_data(vars, (void**)&data) == SUCCESS)) {
				setenv(str, *data, 1);
			}
		}
	}
}
