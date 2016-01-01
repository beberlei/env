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

#ifndef PHP_ENV_H
#define PHP_ENV_H

extern zend_module_entry env_module_entry;
#define phpext_env_ptr &env_module_entry

#define PHP_ENV_VERSION "0.2.0"

#ifdef PHP_WIN32
#	define PHP_ENV_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_ENV_API __attribute__ ((visibility("default")))
#else
#	define PHP_ENV_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(env)
	HashTable *vars;
	char *file;
	int parse_err;
ZEND_END_MODULE_GLOBALS(env)

#ifdef ZTS
#define ENV_G(v) TSRMG(env_globals_id, zend_env_globals *, v)
#else
#define ENV_G(v) (env_globals.v)
#endif

#endif	/* PHP_ENV_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
