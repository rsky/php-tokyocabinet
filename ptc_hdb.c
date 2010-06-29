/*
 * Tokyo Cabinet PHP Bindings - The hash database API of Tokyo Cabinet
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
#include "ptc_abstract.h"
#include "ptc_compat.h"
#include <tchdb.h>

/* {{{ macros */

#define PTC_HDB_METHOD(name) static PHP_METHOD(TCHDB, name)
#define PTC_HDB_ME(name, arg_info) PHP_ME(TCHDB, name, arg_info, ZEND_ACC_PUBLIC)

/* }}} */

/* {{{ globals */

static zend_class_entry *ce_TCHDB;
static zend_object_handlers ptc_hdb_handlers;
static ptc_functions ptc_hdb_functions;

/* }}} */

/* {{{ internal function prototypes */

static zend_object_value
ptc_hdb_new(zend_class_entry *ce TSRMLS_DC);

static void
ptc_hdb_free_storage(void *object TSRMLS_DC);

/* }}} */

/* {{{ TCHDB method prototypes */

PTC_HDB_METHOD(__construct);
PTC_HDB_METHOD(setmutex);
PTC_HDB_METHOD(tune);
PTC_HDB_METHOD(setcache);
PTC_HDB_METHOD(setxmsiz);
PTC_HDB_METHOD(putasync);
PTC_HDB_METHOD(optimize);
PTC_HDB_METHOD(xmsiz);

/* }}} */

/* {{{ TCHDB argument informations */

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_hdb_ctor, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, ktype)
	ZEND_ARG_INFO(0, vtype)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_hdb_errmsg, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, ecode)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_hdb_seterrmode, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, emode)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_hdb_open, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, omode)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_hdb_copy, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_hdb_key, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_hdb_key_num, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, num)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_hdb_key_value, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_hdb_fwmkeys, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, prefix)
	ZEND_ARG_INFO(0, max)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_hdb_tune, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, bnum)
	ZEND_ARG_INFO(0, apow)
	ZEND_ARG_INFO(0, fpow)
	ZEND_ARG_INFO(0, opts)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_hdb_setcache, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, rcnum)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_hdb_setxmsiz, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, xmsiz)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_hdb_optimize, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, bnum)
	ZEND_ARG_INFO(0, apow)
	ZEND_ARG_INFO(0, fpow)
	ZEND_ARG_INFO(0, opts)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ Methods */

/* {{{ void TCHDB::__construct([int ktype[, int vtype]])
 * Create a hash database object.
 */
PTC_HDB_METHOD(__construct)
{
	PTC_DB_DECLARE_OBJECT(obj);
	long ktype = 0L;
	long vtype = 0L;

	if (parse_parameters("|ll", &ktype, &vtype) == FAILURE) {
		return;
	}

	if (obj->db != NULL) {
		ptc_raise_error(obj->emode, PTC_E_INVALID TSRMLS_CC);
		return;
	}
	obj->db = (void *)tchdbnew();

	switch (ktype) {
	case PTC_IS_BINARY: obj->ktype = PTC_KEY_IS_BINARY; break;
	case PTC_IS_INT32: obj->ktype = PTC_KEY_IS_INT32; break;
	case PTC_IS_INT64: obj->ktype = PTC_KEY_IS_INT64; break;
	default: ptc_raise_error(obj->emode, PTC_E_PARAM TSRMLS_CC); return;
	}

	switch (vtype) {
	case PTC_IS_BINARY: obj->vtype = PTC_VALUE_IS_BINARY; break;
	case PTC_IS_SERIALIZED: obj->vtype = PTC_VALUE_IS_SERIALIZED; break;
	default: ptc_raise_error(obj->emode, PTC_E_PARAM TSRMLS_CC); return;
	}
}
/* }}} TCHDB::__construct */

/* {{{ bool TCHDB::tune(int bnum, int apow, int fpow, int opts)
 * Set the tuning parameters of a hash database object.
 */
PTC_HDB_METHOD(tune)
{
	PTC_DB_DECLARE_OBJECT(obj);
	long lbnum, lapow, lfpow, lopts;
	int64_t bnum;
	int8_t apow, fpow;
	uint8_t opts;

	if (parse_parameters("llll", &lbnum, &lapow, &lfpow, &lopts) == FAILURE) {
		return;
	}

	bnum = (int64_t)lbnum;
	apow = (lapow < 0L) ? -1 : (int8_t)(lapow & 0x7FL);
	fpow = (lfpow < 0L) ? -1 : (int8_t)(lfpow & 0x7FL);
	opts = (lopts < 0L) ? 0 : (uint8_t)(lopts & 0xFFL);

	if (tchdbtune((TCHDB *)obj->db, bnum, apow, fpow, opts)) {
		RETURN_TRUE;
	}
	ptc_db_raise_error(obj TSRMLS_CC);
	RETURN_FALSE;
}
/* }}} TCHDB::tune */

/* {{{ bool TCHDB::setcache(int rcnum)
 * Set the caching parameters of a hash database object.
 */
PTC_HDB_METHOD(setcache)
{
	PTC_DB_DECLARE_OBJECT(obj);
	long lrcnum;
	int32_t rcnum;

	if (parse_parameters("l", &lrcnum) == FAILURE) {
		return;
	}

	rcnum = (lrcnum < 0L) ? -1 : (int32_t)(lrcnum & 0x7FFFFFFFL);

	if (!tchdbsetcache((TCHDB *)obj->db, rcnum)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} TCHDB::setcache */

/* {{{ bool TCHDB::setxmsiz(int xmsiz)
 * Set the size of the extra mapped memory of a hash database object.
 */
PTC_HDB_METHOD(setxmsiz)
{
	PTC_DB_DECLARE_OBJECT(obj);
	long lxmsiz;
	int64_t xmsiz;

	if (parse_parameters("l", &lxmsiz) == FAILURE) {
		return;
	}

	xmsiz = (int64_t)lxmsiz;

	if (!tchdbsetxmsiz((TCHDB *)obj->db, xmsiz)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} TCHDB::setxmsiz */

/* {{{ bool TCHDB::putasync(string key, string value)
 * Store a record into a hash database object in asynchronous fashion.
 */
PTC_HDB_METHOD(putasync)
{
	PTC_DB_MN_S(put_impl)(INTERNAL_FUNCTION_PARAM_PASSTHRU,
			(ptc_sput_func_t)tchdbputasync, TCESUCCESS);
}
/* }}} TCHDB::putasync */

/* {{{ bool TCHDB::optimize(int bnum, int apow, int fpow, int opts)
 * Optimize the file of a hash database object.
 */
PTC_HDB_METHOD(optimize)
{
	PTC_DB_DECLARE_OBJECT(obj);
	long lbnum, lapow, lfpow, lopts;
	int64_t bnum;
	int8_t apow, fpow;
	uint8_t opts;

	if (parse_parameters("llll", &lbnum, &lapow, &lfpow, &lopts) == FAILURE) {
		return;
	}

	bnum = (int64_t)lbnum;
	apow = (lapow < 0L) ? -1 : (int8_t)(lapow & 0x7FL);
	fpow = (lfpow < 0L) ? -1 : (int8_t)(lfpow & 0x7FL);
	opts = (lopts < 0L) ? UINT8_MAX : (uint8_t)(lopts & 0xFFL);

	if (tchdboptimize((TCHDB *)obj->db, bnum, apow, fpow, opts)) {
		RETURN_TRUE;
	}
	ptc_db_raise_error(obj TSRMLS_CC);
	RETURN_FALSE;
}
/* }}} TCHDB::optimize */

/* {{{ int TCHDB::xmsiz(void)
 * Get the size of the extra mapped memory of a hash database object.
 */
PTC_HDB_METHOD(xmsiz)
{
	PTC_DB_DECLARE_OBJECT(obj);
	uint64_t xmsiz;

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	xmsiz = tchdbxmsiz((TCHDB *)obj->db);
	RETURN_UINT64(xmsiz);
}
/* }}} TCHDB::xmsiz */

/* }}} Methods */

/* {{{ class_init_TCHDB() */
PTC_LOCAL int
class_init_TCHDB(ptc_deps *deps TSRMLS_DC)
{
	zend_class_entry ce;

	/* {{{ Class registration */

	if (ptc_class_exists("tchdb")) {
		zend_error(E_CORE_WARNING, "Cannot redeclare class TCHDB");
		return FAILURE;
	}

	(void)memcpy(&ptc_hdb_handlers,
			zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	ptc_hdb_handlers.clone_obj = NULL;

	(void)memset(&ptc_hdb_functions, 0 , sizeof(ptc_functions));
	ptc_hdb_functions.dbnew     = (ptc_new_func_t)      tchdbnew;
	ptc_hdb_functions.del       = (ptc_del_func_t)      tchdbdel;
	ptc_hdb_functions.errmsg    = (ptc_errmsg_func_t)   tchdberrmsg;
	ptc_hdb_functions.ecode     = (ptc_ecode_func_t)    tchdbecode;
	ptc_hdb_functions.setmutex  = (ptc_setmutex_func_t) tchdbsetmutex;
	ptc_hdb_functions.open      = (ptc_open_func_t)     tchdbopen;
	ptc_hdb_functions.close     = (ptc_close_func_t)    tchdbclose;
	ptc_hdb_functions.sync      = (ptc_sync_func_t)     tchdbsync;
	ptc_hdb_functions.vanish    = (ptc_vanish_func_t)   tchdbvanish;
	ptc_hdb_functions.copy      = (ptc_copy_func_t)     tchdbcopy;
	ptc_hdb_functions.path      = (ptc_path_func_t)     tchdbpath;
	ptc_hdb_functions.rnum      = (ptc_rnum_func_t)     tchdbrnum;
	ptc_hdb_functions.fsiz      = (ptc_fsiz_func_t)     tchdbfsiz;
	ptc_hdb_functions.iterinit  = (ptc_iterinit_func_t) tchdbiterinit;
	ptc_hdb_functions.fwmkeys   = (ptc_fwmkeys_func_t)  tchdbfwmkeys;
	ptc_hdb_functions.u.s.put       = (ptc_sput_func_t) tchdbput;
	ptc_hdb_functions.u.s.putkeep   = (ptc_sput_func_t) tchdbputkeep;
	ptc_hdb_functions.u.s.putcat    = (ptc_sput_func_t) tchdbputcat;
	ptc_hdb_functions.u.s.out       = (ptc_sout_func_t) tchdbout;
	ptc_hdb_functions.u.s.get       = (ptc_sget_func_t) tchdbget;
	ptc_hdb_functions.u.s.vsiz      = (ptc_svsiz_func_t)    tchdbvsiz;
	ptc_hdb_functions.u.s.addint    = (ptc_saddint_func_t)  tchdbaddint;
	ptc_hdb_functions.u.s.adddouble = (ptc_sadddouble_func_t)   tchdbadddouble;
	ptc_hdb_functions.u.s.iternext  = (ptc_siternext_func_t)    tchdbiternext;

	zend_function_entry TCHDB_methods[] = {
		PHP_ME(TCHDB, __construct, ptc_arg_hdb_ctor, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PTC_DB_ME(ecode,        NULL)
		PTC_DB_ME(errmsg,       ptc_arg_hdb_errmsg)
		PTC_DB_ME(seterrmode,   ptc_arg_hdb_seterrmode)
		PTC_DB_ME(setmutex,     NULL)
		PTC_HDB_ME(tune,        ptc_arg_hdb_tune)
		PTC_HDB_ME(setcache,    ptc_arg_hdb_setcache)
		PTC_HDB_ME(setxmsiz,    ptc_arg_hdb_setxmsiz)
		PTC_DB_ME(open,         ptc_arg_hdb_open)
		PTC_DB_ME(close,        NULL)
		PTC_DB_ME_S(put,        ptc_arg_hdb_key_value)
		PTC_DB_ME_S(putkeep,    ptc_arg_hdb_key_value)
		PTC_DB_ME_S(putcat,     ptc_arg_hdb_key_value)
		PTC_HDB_ME(putasync,    ptc_arg_hdb_key_value)
		PTC_DB_ME_S(out,        ptc_arg_hdb_key)
		PTC_DB_ME_S(get,        ptc_arg_hdb_key)
		PTC_DB_ME_S(vsiz,       ptc_arg_hdb_key)
		PTC_DB_ME(fwmkeys,      ptc_arg_hdb_fwmkeys)
		PTC_DB_ME_S(addint,     ptc_arg_hdb_key_num)
		PTC_DB_ME_S(adddouble,  ptc_arg_hdb_key_num)
		PTC_DB_ME_S(getint,     ptc_arg_hdb_key)
		PTC_DB_ME_S(getdouble,  ptc_arg_hdb_key)
		PTC_DB_ME(sync,         NULL)
		PTC_HDB_ME(optimize,    ptc_arg_hdb_optimize)
		PTC_DB_ME(vanish,       NULL)
		PTC_DB_ME(copy,         ptc_arg_hdb_copy)
		PTC_DB_ME(path,         NULL)
		PTC_DB_ME(rnum,         NULL)
		PTC_DB_ME(fsiz,         NULL)
		PTC_HDB_ME(xmsiz,       NULL)
		/* Iterator implementations */
		PTC_IT_ME(current)
		PTC_IT_ME(key)
		PTC_IT_ME(next)
		PTC_IT_ME(rewind)
		PTC_IT_ME(valid)
		/* ArrayAccess implementations */
		PTC_AA_ME_S(offsetExists,   ptc_arg_hdb_key)
		PTC_DB_MALIAS_S(offsetGet,   get, ptc_arg_hdb_key)
		PTC_DB_MALIAS_S(offsetSet,   put, ptc_arg_hdb_key_value)
		PTC_DB_MALIAS_S(offsetUnset, out, ptc_arg_hdb_key)
		/* Countable implementation */
		PTC_DB_MALIAS(count, rnum, NULL)
		{ NULL, NULL, NULL }
	};

	INIT_CLASS_ENTRY(ce, "TCHDB", TCHDB_methods);
	ce_TCHDB = zend_register_internal_class(&ce TSRMLS_CC);
	if (ce_TCHDB == NULL) {
		return FAILURE;
	}
	ce_TCHDB->create_object = ptc_hdb_new;
	ce_TCHDB->ce_flags |= ZEND_ACC_FINAL_CLASS;

	zend_class_implements(ce_TCHDB TSRMLS_CC, 4, deps->base,
			deps->iterator, deps->arrayaccess, deps->countable);

	/* }}} */

	/* {{{ Constant registration */
#define ptc_hdb_register_constant(name, value) \
	zend_declare_class_constant_long(ce_TCHDB, name, sizeof(name)-1, value TSRMLS_CC)

	ptc_hdb_register_constant("OWRITER",  HDBOWRITER);
	ptc_hdb_register_constant("OREADER",  HDBOREADER);
	ptc_hdb_register_constant("OCREAT",   HDBOCREAT);
	ptc_hdb_register_constant("OTRUNC",   HDBOTRUNC);
	ptc_hdb_register_constant("ONOLCK",   HDBONOLCK);
	ptc_hdb_register_constant("OLCKNB",   HDBOLCKNB);

	ptc_hdb_register_constant("TLARGE",   HDBTLARGE);
	ptc_hdb_register_constant("TDEFLATE", HDBTDEFLATE);
	ptc_hdb_register_constant("TBZIP",    HDBTBZIP);
	ptc_hdb_register_constant("TTCBS",    HDBTTCBS);

	ptc_hdb_register_constant("KTBINARY",     PTC_IS_BINARY);
	ptc_hdb_register_constant("KTSTRING",     PTC_IS_BINARY);
	ptc_hdb_register_constant("KTINT32",      PTC_IS_INT32);
	ptc_hdb_register_constant("KTINT64",      PTC_IS_INT64);

	ptc_hdb_register_constant("VTBINARY",     PTC_IS_BINARY);
	ptc_hdb_register_constant("VTSTRING",     PTC_IS_BINARY);
	ptc_hdb_register_constant("VTSERIALIZED", PTC_IS_SERIALIZED);

#undef ptc_hdb_register_constant
	/* }}} */

	return SUCCESS;
}
/* }}} */

/* {{{ ptc_hdb_new() */
static zend_object_value
ptc_hdb_new(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value retval;
	ptc_db *obj;

	obj = (ptc_db *)ecalloc(1, sizeof(ptc_db));
	obj->type = PTC_HDB;
	obj->numeric_key = 0;
	obj->emode = PTC_ERRMODE_EXCEPTION;
	obj->functions = &ptc_hdb_functions;

	zend_object_std_init(&obj->std, ce TSRMLS_CC);
	object_properties_init(&obj->std, ce);

	retval.handle = zend_objects_store_put(obj,
			(zend_objects_store_dtor_t)zend_objects_destroy_object,
			(zend_objects_free_object_storage_t)ptc_hdb_free_storage,
			NULL TSRMLS_CC);
	retval.handlers = &ptc_hdb_handlers;

	return retval;
}
/* }}} */

/* {{{ ptc_hdb_free_storage() */
static void
ptc_hdb_free_storage(void *object TSRMLS_DC)
{
	ptc_db *obj = (ptc_db *)object;

	if (obj->ikey.str.buf) {
		free(obj->ikey.str.buf);
	}

	if (obj->db) {
		tchdbdel((TCHDB *)obj->db);
	}

	zend_object_std_dtor(&obj->std TSRMLS_CC);

	efree(object);
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
