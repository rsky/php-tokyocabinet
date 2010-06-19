/*
 * Tokyo Cabinet PHP Bindings - The fixed-length database API of Tokyo Cabinet
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
#include <tcfdb.h>

/* {{{ macros */

#define PTC_FDB_METHOD(name) static PHP_METHOD(TCFDB, name)
#define PTC_FDB_ME(name, arg_info) PHP_ME(TCFDB, name, arg_info, ZEND_ACC_PUBLIC)

/* }}} */

/* {{{ globals */

static zend_class_entry *ce_TCFDB;
static zend_object_handlers ptc_fdb_handlers;
static ptc_functions ptc_fdb_functions;

/* }}} */

/* {{{ internal function prototypes */

static zend_object_value
ptc_fdb_new(zend_class_entry *ce TSRMLS_DC);

static void
ptc_fdb_free_storage(void *object TSRMLS_DC);

/* }}} */

/* {{{ TCFDB method prototypes */

PTC_FDB_METHOD(__construct);
PTC_FDB_METHOD(tune);
PTC_FDB_METHOD(range);
PTC_FDB_METHOD(optimize);

/* }}} */

/* {{{ TCFDB argument informations */

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_fdb_ctor, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, vtype)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_fdb_errmsg, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, ecode)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_fdb_seterrmode, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, emode)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_fdb_open, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, omode)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_fdb_copy, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_fdb_id, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_fdb_id_num, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, id)
	ZEND_ARG_INFO(0, num)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_fdb_id_value, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, id)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_fdb_tune, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, limsiz)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_fdb_range, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, interval)
	ZEND_ARG_INFO(0, max)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_fdb_optimize, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, width)
	ZEND_ARG_INFO(0, limsiz)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ Methods */

/* {{{ void TCFDB::__construct([int vtype])
 * Create a fixed-length database object.
 */
PTC_FDB_METHOD(__construct)
{
	PTC_DB_DECLARE_OBJECT(obj);
	long vtype = 0L;

	if (parse_parameters("|l", &vtype) == FAILURE) {
		return;
	}

	if (obj->db != NULL) {
		ptc_raise_error(obj->emode, PTC_E_INVALID TSRMLS_CC);
		return;
	}
	obj->db = (void *)tcfdbnew();

	switch (vtype) {
	case PTC_IS_BINARY: obj->vtype = PTC_VALUE_IS_BINARY; break;
	case PTC_IS_SERIALIZED: obj->vtype = PTC_VALUE_IS_SERIALIZED; break;
	default: ptc_raise_error(obj->emode, PTC_E_PARAM TSRMLS_CC); return;
	}
}
/* }}} TCFDB::__construct */

/* {{{ bool TCFDB::tune(int width, int limsiz)
 * Set the tuning parameters of a fixed-length database object.
 */
PTC_FDB_METHOD(tune)
{
	PTC_DB_DECLARE_OBJECT(obj);
	long lwidth, llimsiz;
	int32_t width;
	int64_t limsiz;

	if (parse_parameters("ll", &lwidth, &llimsiz) == FAILURE) {
		return;
	}

	width = (lwidth < 0L) ? -1 : (int32_t)(lwidth & 0x7FFFFFFFL);
	limsiz = (int64_t)llimsiz;

	if (!tcfdbtune((TCFDB *)obj->db, width, limsiz)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} TCFDB::tune */

/* {{{ array TCFDB::range(string interval[, int max])
 * Get forward matching keys in a fixed-length database object.
 */
PTC_FDB_METHOD(range)
{
	PTC_DB_DECLARE_OBJECT(obj);
	zval **zinterval;
	char *ibuf;
	const void *kbuf;
	int isiz, ksiz, is_copy, i, num;
	long max = -1L;
	TCLIST *keys;

	if (parse_parameters("s|l", &ibuf, &isiz, &max) == FAILURE) {
		return;
	}

	keys = tcfdbrange4((TCFDB *)obj->db, ibuf, isiz, ((max < 0L) ? -1 : (int)max));
	num = tclistnum(keys);
	array_init_size(return_value, (uint)num);

	for (i = 0; i < num; i++) {
		zval *pzv;

		ALLOC_INIT_ZVAL(pzv);
		kbuf = tclistval(keys, i, &ksiz);
		if (ptc_set_zval_key(pzv, kbuf, ksiz, PTC_KEY_IS_ZVAL) == FAILURE) {
			ptc_raise_error(obj->emode, PTC_E_CONVERSION TSRMLS_CC);
			zval_ptr_dtor(&pzv);
			zval_dtor(return_value);
			RETVAL_FALSE;
			break;
		}
		add_next_index_zval(return_value, pzv);
	}

	tclistdel(keys);
}
/* }}} TCFDB::range */

/* {{{ bool TCFDB::optimize(int width, int limsiz)
 * Optimize the file of a fixed-length database object.
 */
PTC_FDB_METHOD(optimize)
{
	PTC_DB_DECLARE_OBJECT(obj);
	long lwidth, llimsiz;
	int32_t width;
	int64_t limsiz;

	if (parse_parameters("ll", &lwidth, &llimsiz) == FAILURE) {
		return;
	}

	width = (lwidth < 0L) ? -1 : (int32_t)(lwidth & 0x7FFFFFFFL);
	limsiz = (int64_t)llimsiz;

	if (!tcfdboptimize((TCFDB *)obj->db, width, llimsiz)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} TCFDB::optimize */

/* }}} Methods */

/* {{{ class_init_TCFDB() */
PTC_LOCAL int
class_init_TCFDB(ptc_deps *deps TSRMLS_DC)
{
	zend_class_entry ce;

	/* {{{ Class registration */

	if (ptc_class_exists("tcfdb")) {
		zend_error(E_CORE_WARNING, "Cannot redeclare class TCFDB");
		return FAILURE;
	}

	(void)memcpy(&ptc_fdb_handlers,
			zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	ptc_fdb_handlers.clone_obj = NULL;

	(void)memset(&ptc_fdb_functions, 0 , sizeof(ptc_functions));
	ptc_fdb_functions.dbnew     = (ptc_new_func_t)  tcfdbnew;
	ptc_fdb_functions.del       = (ptc_del_func_t)  tcfdbdel;
	ptc_fdb_functions.errmsg    = (ptc_errmsg_func_t)   tcfdberrmsg;
	ptc_fdb_functions.ecode     = (ptc_ecode_func_t)    tcfdbecode;
	ptc_fdb_functions.setmutex  = (ptc_setmutex_func_t) tcfdbsetmutex;
	ptc_fdb_functions.open      = (ptc_open_func_t)     tcfdbopen;
	ptc_fdb_functions.close     = (ptc_close_func_t)    tcfdbclose;
	ptc_fdb_functions.sync      = (ptc_sync_func_t)     tcfdbsync;
	ptc_fdb_functions.vanish    = (ptc_vanish_func_t)   tcfdbvanish;
	ptc_fdb_functions.copy      = (ptc_copy_func_t) tcfdbcopy;
	ptc_fdb_functions.path      = (ptc_path_func_t) tcfdbpath;
	ptc_fdb_functions.rnum      = (ptc_rnum_func_t) tcfdbrnum;
	ptc_fdb_functions.fsiz      = (ptc_fsiz_func_t) tcfdbfsiz;
	ptc_fdb_functions.iterinit  = (ptc_iterinit_func_t) tcfdbiterinit;
	//ptc_fdb_functions.fwmkeys   = (ptc_fwmkeys_func_t)  tcfdbfwmkeys;
	ptc_fdb_functions.u.i.put       = (ptc_iput_func_t) tcfdbput;
	ptc_fdb_functions.u.i.putkeep   = (ptc_iput_func_t) tcfdbputkeep;
	ptc_fdb_functions.u.i.putcat    = (ptc_iput_func_t) tcfdbputcat;
	ptc_fdb_functions.u.i.out   = (ptc_iout_func_t) tcfdbout;
	ptc_fdb_functions.u.i.get   = (ptc_iget_func_t) tcfdbget;
	ptc_fdb_functions.u.i.vsiz  = (ptc_ivsiz_func_t)    tcfdbvsiz;
	ptc_fdb_functions.u.i.addint    = (ptc_iaddint_func_t)  tcfdbaddint;
	ptc_fdb_functions.u.i.adddouble = (ptc_iadddouble_func_t)   tcfdbadddouble;
	ptc_fdb_functions.u.i.iternext  = (ptc_iiternext_func_t)    tcfdbiternext;

	zend_function_entry TCFDB_methods[] = {
		PHP_ME(TCFDB, __construct, ptc_arg_fdb_ctor, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PTC_DB_ME(ecode,        NULL)
		PTC_DB_ME(errmsg,       ptc_arg_fdb_errmsg)
		PTC_DB_ME(seterrmode,   ptc_arg_fdb_seterrmode)
		PTC_DB_ME(setmutex,     NULL)
		PTC_FDB_ME(tune,        ptc_arg_fdb_tune)
		PTC_DB_ME(open,         ptc_arg_fdb_open)
		PTC_DB_ME(close,        NULL)
		PTC_DB_ME_I(put,        ptc_arg_fdb_id_value)
		PTC_DB_ME_I(putkeep,    ptc_arg_fdb_id_value)
		PTC_DB_ME_I(putcat,     ptc_arg_fdb_id_value)
		PTC_DB_ME_I(out,        ptc_arg_fdb_id)
		PTC_DB_ME_I(get,        ptc_arg_fdb_id)
		PTC_DB_ME_I(vsiz,       ptc_arg_fdb_id)
		PTC_FDB_ME(range,       ptc_arg_fdb_range)
		PTC_DB_ME_I(addint,     ptc_arg_fdb_id_num)
		PTC_DB_ME_I(adddouble,  ptc_arg_fdb_id_num)
		PTC_DB_ME_I(getint,     ptc_arg_fdb_id)
		PTC_DB_ME_I(getdouble,  ptc_arg_fdb_id)
		PTC_DB_ME(sync,         NULL)
		PTC_FDB_ME(optimize,    ptc_arg_fdb_optimize)
		PTC_DB_ME(vanish,       NULL)
		PTC_DB_ME(copy,         ptc_arg_fdb_copy)
		PTC_DB_ME(path,         NULL)
		PTC_DB_ME(rnum,         NULL)
		PTC_DB_ME(fsiz,         NULL)
		/* Iterator implementations */
		PTC_IT_ME(current)
		PTC_IT_ME(key)
		PTC_IT_ME(next)
		PTC_IT_ME(rewind)
		PTC_IT_ME(valid)
		/* ArrayAccess implementations */
		PTC_AA_ME_I(offsetExists,   ptc_arg_fdb_id)
		PTC_DB_MALIAS_I(offsetGet,   get, ptc_arg_fdb_id)
		PTC_DB_MALIAS_I(offsetSet,   put, ptc_arg_fdb_id_value)
		PTC_DB_MALIAS_I(offsetUnset, out, ptc_arg_fdb_id)
		/* Countable implementation */
		PTC_DB_MALIAS(count, rnum, NULL)
		{ NULL, NULL, NULL }
	};

	INIT_CLASS_ENTRY(ce, "TCFDB", TCFDB_methods);
	ce_TCFDB = zend_register_internal_class(&ce TSRMLS_CC);
	if (ce_TCFDB == NULL) {
		return FAILURE;
	}
	ce_TCFDB->create_object = ptc_fdb_new;
	ce_TCFDB->ce_flags |= ZEND_ACC_FINAL_CLASS;

	zend_class_implements(ce_TCFDB TSRMLS_CC, 4, deps->base,
			deps->iterator, deps->arrayaccess, deps->countable);

	/* }}} */

	/* {{{ Constant registration */
#define ptc_fdb_register_constant(name, value) \
	zend_declare_class_constant_long(ce_TCFDB, name, sizeof(name)-1, value TSRMLS_CC)

	ptc_fdb_register_constant("OWRITER",  FDBOWRITER);
	ptc_fdb_register_constant("OREADER",  FDBOREADER);
	ptc_fdb_register_constant("OCREAT",   FDBOCREAT);
	ptc_fdb_register_constant("OTRUNC",   FDBOTRUNC);
	ptc_fdb_register_constant("ONOLCK",   FDBONOLCK);
	ptc_fdb_register_constant("OLCKNB",   FDBOLCKNB);

	ptc_fdb_register_constant("VTBINARY",     PTC_IS_BINARY);
	ptc_fdb_register_constant("VTSTRING",     PTC_IS_BINARY);
	ptc_fdb_register_constant("VTSERIALIZED", PTC_IS_SERIALIZED);

	ptc_fdb_register_constant("IDMIN",  FDBIDMIN);
	ptc_fdb_register_constant("IDPREV", FDBIDPREV);
	ptc_fdb_register_constant("IDMAX",  FDBIDMAX);
	ptc_fdb_register_constant("IDNEXT", FDBIDNEXT);

#undef ptc_fdb_register_constant
	/* }}} */

	return SUCCESS;
}
/* }}} */

/* {{{ ptc_fdb_new() */
static zend_object_value
ptc_fdb_new(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value retval;
	ptc_db *obj;

	obj = (ptc_db *)ecalloc(1, sizeof(ptc_db));
	obj->type = PTC_FDB;
	obj->numeric_key = 1;
	obj->emode = PTC_ERRMODE_EXCEPTION;
	obj->functions = &ptc_fdb_functions;
	obj->parse_id = tcfdbkeytoid;

	zend_object_std_init(&obj->std, ce TSRMLS_CC);
	zend_hash_copy(obj->std.properties, &ce->default_properties,
			(copy_ctor_func_t)zval_add_ref, NULL, sizeof(zval *));

	retval.handle = zend_objects_store_put(obj,
			(zend_objects_store_dtor_t)zend_objects_destroy_object,
			(zend_objects_free_object_storage_t)ptc_fdb_free_storage,
			NULL TSRMLS_CC);
	retval.handlers = &ptc_fdb_handlers;

	return retval;
}
/* }}} */

/* {{{ ptc_fdb_free_storage() */
static void
ptc_fdb_free_storage(void *object TSRMLS_DC)
{
	ptc_db *obj = (ptc_db *)object;

	if (obj->db) {
		tcfdbdel((TCFDB *)obj->db);
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
