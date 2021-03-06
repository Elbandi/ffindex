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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include <ffindex.h>
#include "php_ffindex.h"

/* If you declare any globals in php_ffindex.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(ffindex)
*/

/* True global resources - no need for thread safety here */
//static int le_ffindex;

/* {{{ FFINDEX_FUNCTIONS_ARG_INFO
 */
FFINDEX_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_build, 0, 0, 3)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, files)
	ZEND_ARG_INFO(0, sorted)
ZEND_END_ARG_INFO()

FFINDEX_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_list, 0, 0, 1)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

FFINDEX_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_get, 0, 0, 3)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

FFINDEX_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_unpack, 0, 0, 3)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, outdir)
ZEND_END_ARG_INFO()

/* }}} */


/* {{{ ffindex_functions[]
 *
 * Every user visible function must have an entry in ffindex_functions[].
 */
const zend_function_entry ffindex_functions[] = {
	PHP_FE(confirm_ffindex_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(ffindex_build, arginfo_build)
	PHP_FE(ffindex_list, arginfo_list)
	PHP_FE(ffindex_get, arginfo_get)
	PHP_FE(ffindex_unpack, arginfo_unpack)
	{NULL, NULL, NULL}	/* Must be the last line in ffindex_functions[] */
};
/* }}} */

/* {{{ ffindex_module_entry
 */
zend_module_entry ffindex_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"ffindex",
	ffindex_functions,
	PHP_MINIT(ffindex),
	PHP_MSHUTDOWN(ffindex),
	PHP_RINIT(ffindex),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(ffindex),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(ffindex),
#if ZEND_MODULE_API_NO >= 20010901
	FFINDEX_VERSION_STR,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_FFINDEX
ZEND_GET_MODULE(ffindex)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("ffindex.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_ffindex_globals, ffindex_globals)
    STD_PHP_INI_ENTRY("ffindex.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_ffindex_globals, ffindex_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_ffindex_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_ffindex_init_globals(zend_ffindex_globals *ffindex_globals)
{
	ffindex_globals->global_value = 0;
	ffindex_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(ffindex)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(ffindex)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(ffindex)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(ffindex)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(ffindex)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "ffindex support", "enabled");
	php_info_print_table_row(2, "version", FFINDEX_VERSION_STR);
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_ffindex_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_ffindex_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "ffindex", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */

int ffindex_insert(FILE *data_file, FILE *index_file, size_t *start_offset, char *input_name) {
	struct stat sb;
	if(stat(input_name, &sb) != -1) {
		if(S_ISDIR(sb.st_mode)) {
			return ffindex_insert_dir(data_file, index_file, start_offset, input_name) == FFINDEX_OK ? SUCCESS : FAILURE;
		}
		else if(S_ISREG(sb.st_mode)) {
			return ffindex_insert_file(data_file, index_file, start_offset, input_name, basename(input_name)) == FFINDEX_OK ? SUCCESS : FAILURE;
		}
	}
	return FAILURE;
}

int ffindex_sort_index(char *index_filename, FILE *index_file) {
	int ret = FAILURE;
	rewind(index_file);
	ffindex_index_t* index = ffindex_index_parse(index_file, 0);
	if(index == NULL)
	{
		return FAILURE;
	}
	if(index->n_entries != 0) {
		ffindex_sort_index_file(index);
		if (ftruncate(fileno(index_file), 0) == 0) {
			if (ffindex_write(index, index_file) == FFINDEX_OK) {
				ret = SUCCESS;
			}
		}
	}
	ffindex_index_free(index);
	return ret;
}


/* procedural APIs*/
/* {{{ proto ffindex_build(string $data_file, string $index_file, array $files[, bool $sort = false]]]])
*/
PHP_FUNCTION(ffindex_build) {
	char *index_file_name;
	int index_file_len;
	FILE *index_file;
	char *data_file_name;
	int data_file_len;
	FILE *data_file;
	zval *zfiles = NULL;
	zend_bool sort = 0;
	size_t offset = 0;

	/* Parse arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssz|b", &data_file_name, &data_file_len, &index_file_name, &index_file_len, &zfiles, &sort) == FAILURE) {
		return;
	}
	if (!data_file_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "data filename cannot be empty");
		RETURN_FALSE;
	}
	if (!index_file_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "index filename cannot be empty");
		RETURN_FALSE;
	}
	if (Z_TYPE_P(zfiles) != IS_STRING && Z_TYPE_P(zfiles) != IS_ARRAY) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "files is neither a string nor an array");
		RETURN_FALSE;
	}
	if (index_file_len == data_file_len && strcmp(index_file_name, data_file_name) == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "index and data file name sould different");
		RETURN_FALSE;
	}

	data_file = php_fopen_with_path(data_file_name, "wb", ".", NULL TSRMLS_CC);
	if (!data_file) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to open '%s' for writing", data_file_name);
		RETURN_FALSE;
	}
	index_file = php_fopen_with_path(index_file_name, "w+b", ".", NULL TSRMLS_CC);
	if (!index_file) {
		fclose(data_file);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to open '%s' for writing", index_file_name);
		RETURN_FALSE;
	}
	RETVAL_TRUE;
	if (Z_TYPE_P(zfiles) == IS_STRING) {
		if (OPENBASEDIR_CHECKPATH(Z_STRVAL_P(zfiles))) {
			RETVAL_FALSE;
		}
		if (ffindex_insert(data_file, index_file, &offset, Z_STRVAL_P(zfiles)) == FAILURE) {
			RETVAL_FALSE;
		}
	} else if (Z_TYPE_P(zfiles) == IS_ARRAY) {
		HashPosition   pos;
		zval         **tmp;
		HashTable *hfiles = Z_ARRVAL_P(zfiles);
		for (zend_hash_internal_pointer_reset_ex(hfiles, &pos);
			zend_hash_get_current_data_ex(hfiles, (void **) &tmp, &pos) == SUCCESS;
			zend_hash_move_forward_ex(hfiles, &pos)) {
			if (Z_TYPE_PP(tmp) != IS_STRING) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Couldn't get string from file lists");
				RETVAL_FALSE;
				break;
			}
			if (OPENBASEDIR_CHECKPATH(Z_STRVAL_PP(tmp))) {
				RETVAL_FALSE;
				break;
			}
			if (ffindex_insert(data_file, index_file, &offset, Z_STRVAL_PP(tmp)) == FAILURE) {
				RETVAL_FALSE;
				break;
			}
		}
	}
	if (Z_LVAL_P(return_value) == 1 && sort == 1) {
		if (ffindex_sort_index(index_file_name, index_file) == FAILURE) {
			RETVAL_FALSE;
		}
	}
	fclose(index_file);
	fclose(data_file);
	if (Z_LVAL_P(return_value) != 1) {
		VCWD_UNLINK(index_file_name);
		VCWD_UNLINK(data_file_name);
	}
}
/* }}} */

/* procedural APIs*/
/* {{{ proto ffindex_list(string $index_file)
*/
PHP_FUNCTION(ffindex_list) {
	char *index_file_name;
	int index_file_len;
	FILE *index_file;

	/* Parse arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &index_file_name, &index_file_len) == FAILURE) {
		return;
	}
	if (!index_file_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "index filename cannot be empty");
		RETURN_FALSE;
	}
	index_file = php_fopen_with_path(index_file_name, "rb", ".", NULL TSRMLS_CC);
	if (!index_file) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to open '%s' for reading", index_file_name);
		RETURN_FALSE;
	}
	ffindex_index_t* index = ffindex_index_parse(index_file, 0);
	if (index != NULL) {
		size_t i;
		zval *tmp;
		array_init(return_value);
		for(i = 0; i < index->n_entries; i++)
		{
			MAKE_STD_ZVAL(tmp);
			array_init(tmp);
			add_assoc_string(tmp, "name", index->entries[i].name, 0);
			add_assoc_long(tmp, "length", index->entries[i].length - 1);
			add_assoc_long(tmp, "mtime", index->entries[i].mtime);
			add_next_index_zval(return_value, tmp);
		}
		free(index);
	} else {
		RETVAL_FALSE;
	}
	fclose(index_file);
}
	
/* procedural APIs*/
/* {{{ proto ffindex_get(string $data_file, string $index_file, string $name)
*/
PHP_FUNCTION(ffindex_get) {
	char *index_file_name;
	int index_file_len;
	FILE *index_file;
	char *data_file_name;
	int data_file_len;
	FILE *data_file;
	char *file_name;
	int file_len;

	/* Parse arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &data_file_name, &data_file_len, &index_file_name, &index_file_len, &file_name, &file_len) == FAILURE) {
		return;
	}
	if (!data_file_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "data filename cannot be empty");
		RETURN_FALSE;
	}
	if (!index_file_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "index filename cannot be empty");
		RETURN_FALSE;
	}
	if (index_file_len == data_file_len && strcmp(index_file_name, data_file_name) == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "index and data file name sould different");
		RETURN_FALSE;
	}
	if (!file_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "filename cannot be empty");
		RETURN_FALSE;
	}

	data_file = php_fopen_with_path(data_file_name, "rb", ".", NULL TSRMLS_CC);
	if (!data_file) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to open '%s' for reading", data_file_name);
		RETURN_FALSE;
	}
	index_file = php_fopen_with_path(index_file_name, "rb", ".", NULL TSRMLS_CC);
	if (!index_file) {
		fclose(data_file);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to open '%s' for reading", index_file_name);
		RETURN_FALSE;
	}
	size_t data_size;
	char *data = ffindex_mmap_data(data_file, &data_size);
	if (data == MAP_FAILED || data == NULL) {
		fclose(index_file);
		fclose(data_file);
		RETURN_FALSE;
	}
	RETVAL_FALSE;
	do {
		ffindex_index_t* index = ffindex_index_parse(index_file, 0);
		if (index == NULL) {
			break;
		}
		ffindex_entry_t* entry = ffindex_bsearch_get_entry(index, file_name);
		if (entry == NULL) {
			break;
		}
		char *filedata = ffindex_get_data_by_entry(data, entry);
		if(filedata == NULL) {
			break;
		}
		RETVAL_STRINGL(filedata, entry->length - 1, 1);
		/*
		if (PG(magic_quotes_runtime)) {
		Z_STRVAL_P(return_value) = php_addslashes(Z_STRVAL_P(return_value),
			Z_STRLEN_P(return_value), &Z_STRLEN_P(return_value), 1 TSRMLS_CC);
		}
		*/
	} while(0);
	fclose(index_file);
	fclose(data_file);
}
/* }}} */

/* procedural APIs*/
/* {{{ proto ffindex_unpack(string $data_file, string $index_file, string $outdir)
*/
PHP_FUNCTION(ffindex_unpack) {
	char *index_file_name;
	int index_file_len;
	FILE *index_file;
	char *data_file_name;
	int data_file_len;
	FILE *data_file;
	char *outdir_name;
	int outdir_len;

	/* Parse arguments */
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sss", &data_file_name, &data_file_len, &index_file_name, &index_file_len, &outdir_name, &outdir_len) == FAILURE) {
		return;
	}
	if (!data_file_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "data filename cannot be empty");
		RETURN_FALSE;
	}
	if (!index_file_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "index filename cannot be empty");
		RETURN_FALSE;
	}
	if (index_file_len == data_file_len && strcmp(index_file_name, data_file_name) == 0) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "index and data file name sould different");
		RETURN_FALSE;
	}
	if (!outdir_len) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "outdir cannot be empty");
		RETURN_FALSE;
	}
	if (OPENBASEDIR_CHECKPATH(outdir_name)) {
		RETURN_FALSE;
	}

	data_file = php_fopen_with_path(data_file_name, "rb", ".", NULL TSRMLS_CC);
	if (!data_file) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to open '%s' for reading", data_file_name);
		RETURN_FALSE;
	}
	index_file = php_fopen_with_path(index_file_name, "rb", ".", NULL TSRMLS_CC);
	if (!index_file) {
		fclose(data_file);
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to open '%s' for reading", index_file_name);
		RETURN_FALSE;
	}
	size_t data_size;
	char *data = ffindex_mmap_data(data_file, &data_size);
	if (data == MAP_FAILED || data == NULL) {
		fclose(index_file);
		fclose(data_file);
		RETURN_FALSE;
	}
	size_t count = 0;
	do {
		ffindex_index_t* index = ffindex_index_parse(index_file, 0);
		if (index == NULL) {
			break;
		}
		size_t range_start = 0;
		size_t range_end = index->n_entries;
		FILE *output_file;
		size_t entry_index;
		for(entry_index = range_start; entry_index < range_end; entry_index++)
		{
			ffindex_entry_t* entry = ffindex_get_entry_by_index(index, entry_index);
			if(entry == NULL) {
				continue;
			}
			char *filedata = ffindex_get_data_by_entry(data, entry);
			if(filedata == NULL) {
				continue;
			}
			output_file = php_fopen_with_path(entry->name, "w", outdir_name, NULL TSRMLS_CC);
			if(output_file == NULL) {
				continue;
			}
			size_t written = fwrite(filedata, entry->length - 1, 1, output_file);
			if(written >= 1) {
				count++;
			}
			fclose(output_file);
		}
	} while(0);
	fclose(index_file);
	fclose(data_file);
	RETVAL_LONG(count);
}
/* }}} */

/*
if (strlen(filename) != filename_len) {
		RETURN_FALSE;
	}
*/

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
