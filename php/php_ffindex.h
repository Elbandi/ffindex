/*
  +----------------------------------------------------------------------+
  | PHP Version 5														 |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2012 The PHP Group								 |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Andras Elso	   <elso.andras@gmail.com>						 |
  +----------------------------------------------------------------------+
*/

/* $Id */

#ifndef PHP_FFINDEX_H
#define PHP_FFINDEX_H

extern zend_module_entry ffindex_module_entry;
#define phpext_ffindex_ptr &ffindex_module_entry

#ifdef PHP_WIN32
#	define PHP_FFINDEX_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_FFINDEX_API __attribute__ ((visibility("default")))
#else
#	define PHP_FFINDEX_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#if (PHP_MAJOR_VERSION == 5) && (PHP_MINOR_VERSION > 2)
#define FFINDEX_ARG_PREFIX
#else
#define FFINDEX_ARG_PREFIX static
#endif

#define STR1(x)  #x
#define STR(x)  STR1(x)
#define FFINDEX_VERSION_STR STR(FFINDEX_VERSION)

PHP_MINIT_FUNCTION(ffindex);
PHP_MSHUTDOWN_FUNCTION(ffindex);
PHP_RINIT_FUNCTION(ffindex);
PHP_RSHUTDOWN_FUNCTION(ffindex);
PHP_MINFO_FUNCTION(ffindex);

PHP_FUNCTION(confirm_ffindex_compiled);	/* For testing, remove later. */
PHP_FUNCTION(ffindex_build);
PHP_FUNCTION(ffindex_get);
PHP_FUNCTION(ffindex_unpack);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(ffindex)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(ffindex)
*/

/* In every utility function you add that needs to use variables 
   in php_ffindex_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as FFINDEX_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define FFINDEX_G(v) TSRMG(ffindex_globals_id, zend_ffindex_globals *, v)
#else
#define FFINDEX_G(v) (ffindex_globals.v)
#endif

#endif	/* PHP_FFINDEX_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet expandtab sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */

