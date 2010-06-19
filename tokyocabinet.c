/*
 * Tokyo Cabinet PHP Bindings
 *
 * Copyright (c) 2007-2010 Ryusuke SEKIYAMA. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * @package     php_tokyocabinet
 * @author      Ryusuke SEKIYAMA <rsky0711@gmail.com>
 * @copyright   2007-2010 Ryusuke SEKIYAMA
 * @license     http://www.opensource.org/licenses/mit-license.php  MIT License
 * @version     SVN: $Id$
 */

#include "php_tokyocabinet.h"
#if PTC_ENABLE_DYSTOPIA
#include <tcqdb.h>
#endif
#if PTC_ENABLE_TYRANT
#include <ttutil.h>
#endif

/* {{{ globals */

static zend_class_entry *ce_TCDB;

/* }}} */

/* {{{ module function prototypes */

static PHP_MINIT_FUNCTION(tokyocabinet);
static PHP_MINFO_FUNCTION(tokyocabinet);

/* }}} */

/* {{{ internal function prototypes */

static int
class_init_TCDB(TSRMLS_D);

static int
ptc_format_version(char *buf, size_t siz, int32_t vernum);

/* }}} */

/* {{{ TCDB argument informations */

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_db_errmsg, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, ecode)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_db_seterrmode, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, emode)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ cross-extension dependencies */
static zend_module_dep tokyocabinet_deps[] = {
	ZEND_MOD_REQUIRED("spl")
	{NULL, NULL, NULL, 0}
};
/* }}} */

/* {{{ tokyocabinet_module_entry */
static zend_module_entry tokyocabinet_module_entry = {
	STANDARD_MODULE_HEADER_EX,
	NULL,
	tokyocabinet_deps,
	"tokyocabinet",
	NULL,
	PHP_MINIT(tokyocabinet),
	NULL,
	NULL,
	NULL,
	PHP_MINFO(tokyocabinet),
	PTC_MODULE_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_TOKYOCABINET
ZEND_GET_MODULE(tokyocabinet)
#endif

/* {{{ PHP_MINIT_FUNCTION */
static PHP_MINIT_FUNCTION(tokyocabinet)
{
	zend_class_entry **tmp;
	ptc_deps deps;

	/* {{{ Constant registration */
#define ptc_register_constant(name, value) \
	REGISTER_LONG_CONSTANT(name, value, CONST_PERSISTENT | CONST_CS);
#define ptc_register_string_constant(name, value) \
	REGISTER_STRINGL_CONSTANT(name, (char *)value, strlen(value), \
			CONST_PERSISTENT | CONST_CS)

	/* Linked library version number strings */
	ptc_register_string_constant("TCVERSION", tcversion);
#if PTC_ENABLE_DYSTOPIA
	ptc_register_string_constant("TDVERSION", tdversion);
#endif
#if PTC_ENABLE_TYRANT
	ptc_register_string_constant("TTVERSION", ttversion);
#endif

	/* Compiled library version numbers */
	ptc_register_constant("TCVERNUM", TOKYOCABINET_VERSION_NUMBER);
#if PTC_ENABLE_DYSTOPIA
	ptc_register_constant("TDVERNUM", TOKYODYSTOPIA_VERSION_NUMBER);
#endif
#if PTC_ENABLE_TYRANT
	ptc_register_constant("TTVERNUM", TOKYOTYRANT_VERSION_NUMBER);
#endif

	/* Tokyo Cabinet's error codes */
	ptc_register_constant("TCESUCCESS", TCESUCCESS);
	ptc_register_constant("TCETHREAD",  TCETHREAD);
	ptc_register_constant("TCEINVALID", TCEINVALID);
	ptc_register_constant("TCENOFILE",  TCENOFILE);
	ptc_register_constant("TCENOPERM",  TCENOPERM);
	ptc_register_constant("TCEMETA",    TCEMETA);
	ptc_register_constant("TCERHEAD",   TCERHEAD);
	ptc_register_constant("TCEOPEN",    TCEOPEN);
	ptc_register_constant("TCECLOSE",   TCECLOSE);
	ptc_register_constant("TCETRUNC",   TCETRUNC);
	ptc_register_constant("TCESYNC",    TCESYNC);
	ptc_register_constant("TCESTAT",    TCESTAT);
	ptc_register_constant("TCESEEK",    TCESEEK);
	ptc_register_constant("TCEREAD",    TCEREAD);
	ptc_register_constant("TCEWRITE",   TCEWRITE);
	ptc_register_constant("TCEMMAP",    TCEMMAP);
	ptc_register_constant("TCELOCK",    TCELOCK);
	ptc_register_constant("TCEUNLINK",  TCEUNLINK);
	ptc_register_constant("TCERENAME",  TCERENAME);
	ptc_register_constant("TCEMKDIR",   TCEMKDIR);
	ptc_register_constant("TCERMDIR",   TCERMDIR);
	ptc_register_constant("TCEKEEP",    TCEKEEP);
	ptc_register_constant("TCENOREC",   TCENOREC);
	ptc_register_constant("TCEMISC",    TCEMISC);

	/* Module's error codes */
	ptc_register_constant("TCMEINVALID",     PTC_E_INVALID);
	ptc_register_constant("TCMEPARAM",       PTC_E_PARAM);
	ptc_register_constant("TCMEUNSUPPORTED", PTC_E_UNSUPPORTED);
	ptc_register_constant("TCMECONVERSION",  PTC_E_CONVERSION);
	ptc_register_constant("TCMEMISC",        PTC_E_MISC);

#undef ptc_register_constant
#undef ptc_register_string_constant
	/* }}} */

	/* {{{ Get interfaces */
#define ptc_get_class(name) \
	if (ptc_class_find(#name, &tmp) == FAILURE) { \
		zend_error(E_WARNING, "Class '" #name "' not found"); \
		return FAILURE; \
	} else { \
		deps.name = *tmp; \
	}

	ptc_get_class(iterator);
	ptc_get_class(iteratoraggregate);
	ptc_get_class(arrayaccess);
	ptc_get_class(countable);

#undef ptc_get_class
	/* }}} */

	/* {{{ Class registration */

	if (FAILURE == class_init_TCDB(TSRMLS_C)) {
		return FAILURE;
	}

	deps.base = ce_TCDB;

	if (FAILURE == class_init_TCHDB(&deps TSRMLS_CC) || // hash database
	    FAILURE == class_init_TCBDB(&deps TSRMLS_CC) || // B+ tree database
	    FAILURE == class_init_TCFDB(&deps TSRMLS_CC) || // fixed-length database
#if PTC_ENABLE_DYSTOPIA
	    FAILURE == class_init_TCIDB(&deps TSRMLS_CC) || // indexed database `Dystopia'
	    FAILURE == class_init_TCJDB(&deps TSRMLS_CC) || // tagged database `Laputa'
#endif
#if PTC_ENABLE_TYRANT
	    FAILURE == class_init_TCRDB(&deps TSRMLS_CC) || // network interface `Tyrant'
#endif
	    FAILURE == class_init_TCUtil(TSRMLS_C) ||       // utilities
	    FAILURE == class_init_TCException(TSRMLS_C))    // exception
	{
		return FAILURE;
	}

	/* }}} */

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION */
static PHP_MINFO_FUNCTION(tokyocabinet)
{
	char version[32];

	php_info_print_table_start();
	php_info_print_table_row(2, "Tokyo Cabinet Support", "enabled");
#if PTC_ENABLE_DYSTOPIA
	php_info_print_table_row(2, "Tokyo Dystopia Support", "enabled");
#else
	php_info_print_table_row(2, "Tokyo Dystopia Support", "disabled");
#endif
#if PTC_ENABLE_TYRANT
	php_info_print_table_row(2, "Tokyo Tyrant Support", "enabled");
#else
	php_info_print_table_row(2, "Tokyo Tyrant Support", "disabled");
#endif
	php_info_print_table_row(2, "Module Version", PTC_MODULE_VERSION);
	(void)ptc_format_version(version, 32, TOKYOCABINET_VERSION_NUMBER);
	php_info_print_table_row(2, "TC Version (linked)", tcversion);
	php_info_print_table_row(2, "TC Version (compiled)", version);
#if PTC_ENABLE_DYSTOPIA
	(void)ptc_format_version(version, 32, TOKYODYSTOPIA_VERSION_NUMBER);
	php_info_print_table_row(2, "TD Version (linked)", tdversion);
	php_info_print_table_row(2, "TD Version (compiled)", version);
#endif
#if PTC_ENABLE_TYRANT
	(void)ptc_format_version(version, 32, TOKYOTYRANT_VERSION_NUMBER);
	php_info_print_table_row(2, "TT Version (linked)", ttversion);
	php_info_print_table_row(2, "TT Version (compiled)", version);
#endif
	php_info_print_table_end();
}
/* }}} */

/* {{{ class_init_TCDB() */
static int
class_init_TCDB(TSRMLS_D)
{
	zend_class_entry ce;

	zend_function_entry TCDB_methods[] = {
		ZEND_ABSTRACT_ME(TCDB, ecode,       NULL)
		ZEND_ABSTRACT_ME(TCDB, errmsg,      ptc_arg_db_errmsg)
		ZEND_ABSTRACT_ME(TCDB, seterrmode,  ptc_arg_db_seterrmode)
		{ NULL, NULL, NULL }
	};

	/* {{{ Class registration */

	if (ptc_class_exists("tcdb")) {
		zend_error(E_CORE_WARNING, "Cannot redeclare class TCDB");
		return FAILURE;
	}

	INIT_CLASS_ENTRY(ce, "TCDB", TCDB_methods);
	ce_TCDB = zend_register_internal_interface(&ce TSRMLS_CC);
	if (ce_TCDB == NULL) {
		return FAILURE;
	}

	/* }}} */

	/* {{{ Constant registration */
#define ptc_db_register_constant(name, value) \
	zend_declare_class_constant_long(ce_TCDB, name, sizeof(name)-1, value TSRMLS_CC)

	/* Tokyo Cabinet's error codes */
	ptc_db_register_constant("ESUCCESS",    TCESUCCESS);
	ptc_db_register_constant("ETHREAD",     TCETHREAD);
	ptc_db_register_constant("EINVALID",    TCEINVALID);
	ptc_db_register_constant("ENOFILE",     TCENOFILE);
	ptc_db_register_constant("ENOPERM",     TCENOPERM);
	ptc_db_register_constant("EMETA",       TCEMETA);
	ptc_db_register_constant("ERHEAD",      TCERHEAD);
	ptc_db_register_constant("EOPEN",       TCEOPEN);
	ptc_db_register_constant("ECLOSE",      TCECLOSE);
	ptc_db_register_constant("ETRUNC",      TCETRUNC);
	ptc_db_register_constant("ESYNC",       TCESYNC);
	ptc_db_register_constant("ESTAT",       TCESTAT);
	ptc_db_register_constant("ESEEK",       TCESEEK);
	ptc_db_register_constant("EREAD",       TCEREAD);
	ptc_db_register_constant("EWRITE",      TCEWRITE);
	ptc_db_register_constant("EMMAP",       TCEMMAP);
	ptc_db_register_constant("ELOCK",       TCELOCK);
	ptc_db_register_constant("EUNLINK",     TCEUNLINK);
	ptc_db_register_constant("ERENAME",     TCERENAME);
	ptc_db_register_constant("EMKDIR",      TCEMKDIR);
	ptc_db_register_constant("ERMDIR",      TCERMDIR);
	ptc_db_register_constant("EKEEP",       TCEKEEP);
	ptc_db_register_constant("ENOREC",      TCENOREC);
	ptc_db_register_constant("EMISC",       TCEMISC);
                              
	/* Module's error codes */
	ptc_db_register_constant("MEINVALID",       PTC_E_INVALID);
	ptc_db_register_constant("MEPARAM",         PTC_E_PARAM);
	ptc_db_register_constant("MEUNSUPPORTED",   PTC_E_UNSUPPORTED);
	ptc_db_register_constant("MECONVERSION",    PTC_E_CONVERSION);
	ptc_db_register_constant("MEMISC",          PTC_E_MISC);

	/* Error handling modes */
	ptc_db_register_constant("EMSILENT",    PTC_ERRMODE_SILENT);
	ptc_db_register_constant("EMWARNING",   PTC_ERRMODE_WARNING);
	ptc_db_register_constant("EMEXCEPTION", PTC_ERRMODE_EXCEPTION);
#undef ptc_bdb_register_constant
	/* }}} */

	return SUCCESS;
}
/* }}} */

/* {{{ ptc_format_version() */
static int
ptc_format_version(char *buf, size_t siz, int32_t vernum)
{
	int major, minor, release;

	major   = (int)(vernum / 1000000);
	minor   = (int)(vernum / 1000 - major * 1000);
	release = (int)(vernum - major * 1000000 - minor * 1000);

	return snprintf(buf, siz, "%d.%d.%d", major, minor, release);
}
/* }}} */

/* {{{ ptc_errmsg() */
PTC_LOCAL const char *
ptc_errmsg(int ecode)
{
	switch (ecode) {
	case PTC_E_INVALID:
		return "invalid operation";
	case PTC_E_PARAM:
		return "invalid parameter";
	case PTC_E_CONVERSION:
		return "value conversion failure";
	case PTC_E_UNSUPPORTED:
		return "unsuppoted feature specified";
	case PTC_E_MISC:
		return "miscellaneous error";
	default:
		return "unknown error";
	}
}
/* }}} */

/* {{{ ptc_raise_error() */
PTC_LOCAL void
ptc_raise_error(ptc_emode emode, int ecode TSRMLS_DC)
{
	ptc_raise_error_ex(emode, ecode, ptc_errmsg(ecode) TSRMLS_CC);
}
/* }}} */

/* {{{ ptc_raise_error_ex() */
PTC_LOCAL void
ptc_raise_error_ex(ptc_emode emode, int ecode, const char *errmsg TSRMLS_DC)
{
	switch (emode) {
	case PTC_ERRMODE_EXCEPTION:
		throw_exception(ptc_get_ce_tcexception(), ecode, "%s", errmsg);
	case PTC_ERRMODE_SILENT:
		return;
	case PTC_ERRMODE_WARNING:
	default:
		raise_error(E_WARNING, "%d: %s", ecode, errmsg);
	}
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
