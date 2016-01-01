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
#include "env.h"

ZEND_DECLARE_MODULE_GLOBALS(env)

/* True global resources - no need for thread safety here */
static int le_env;

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("env.file", "", PHP_INI_ALL, OnUpdateString, file, zend_env_globals, env_globals)
PHP_INI_END()
/* }}} */


#if PHP_VERSION_ID < 70000
void char_ptr_dtor(char **str)
{
	free(*str);
}
#else
#define char_ptr_dtor ZVAL_PTR_DTOR
#endif

/* {{{ PHP_GINIT_FUNCTION
 */
PHP_GINIT_FUNCTION(env)
{
	env_globals->file = NULL;
	env_globals->parse_err = 0;
	env_globals->vars = (HashTable*)pemalloc(sizeof(HashTable), 1);
	zend_hash_init(env_globals->vars, 128, NULL, char_ptr_dtor, 1);
}
/* }}} */

/* {{{ PHP_GSHUTDOWN_FUNCTION
 */
PHP_GSHUTDOWN_FUNCTION(env)
{
	env_globals->file = NULL;
	env_globals->parse_err = 0;
	free(env_globals->vars);
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(env)
{
	REGISTER_INI_ENTRIES();

	php_env_module_init(ENV_G(vars) TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(env)
{
	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(env)
{
	php_env_request_init(ENV_G(vars) TSRMLS_CC);

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
	PHP_MODULE_GLOBALS(env),   /* globals descriptor */
	PHP_GINIT(env),            /* globals ctor */
	PHP_GSHUTDOWN(env),        /* globals dtor */
	NULL,                      /* post deactivate */
	STANDARD_MODULE_PROPERTIES_EX
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
