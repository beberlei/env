#include "php.h"
#include "../php_env.h"
#include "../env.h"

ZEND_DECLARE_MODULE_GLOBALS(env)

static void php_env_ini_parser_cb(zval *key, zval *value, zval *index, int callback_type, void *arg) /* {{{ */ {

	if (ENV_G(parse_err)) {
		return;
	}

	if (value == NULL) {
		return;
	}

	if (callback_type == ZEND_INI_PARSER_ENTRY) {
		setenv(Z_STRVAL_P(key), Z_STRVAL_P(value), 1);
	} else if (callback_type == ZEND_INI_PARSER_SECTION || callback_type == ZEND_INI_PARSER_POP_ENTRY) {
		ENV_G(parse_err) = 1;
	}
}

int php_env_module_init(TSRMLS_D) {
	int ndir = 255;
	uint32_t i;
	unsigned char c;
	struct zend_stat sb;
	zend_file_handle fh = {0};

	if (ENV_G(file) != NULL && strlen(ENV_G(file)) > 0 && VCWD_STAT(ENV_G(file), &sb) == 0) {
		if (S_ISREG(sb.st_mode)) {
			if ((fh.handle.fp = VCWD_FOPEN(ENV_G(file), "r"))) {
				fh.filename = ENV_G(file);
				fh.type = ZEND_HANDLE_FP;

				if (zend_parse_ini_file(&fh, 0, 0 /* ZEND_INI_SCANNER_NORMAL */,
							php_env_ini_parser_cb, NULL) == FAILURE || ENV_G(parse_err)) {
					if (ENV_G(parse_err)) {
						php_error(E_WARNING, "env: parsing '%s' failed", ENV_G(file));
					}

					ENV_G(parse_err) = 0;

					return SUCCESS;
				}
			}
		}
	}

	return SUCCESS;
}
