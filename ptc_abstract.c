/*
 * Tokyo Cabinet PHP Bindings - Abstract API
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

#include "ptc_abstract.h"

/* {{{ ptc_db_raise_error()
 */
PTC_LOCAL void
ptc_db_raise_error(ptc_db *obj TSRMLS_DC)
{
	int ecode = PTC_ECODE(obj);
	ptc_raise_error_ex(obj->emode, ecode, obj->functions->errmsg(ecode) TSRMLS_CC);
}
/* }}} */

/* {{{ int TC*DB::ecode(void)
 * Get the last happened error code.
 */
PTC_DB_METHOD(ecode)
{
	PTC_DB_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	RETURN_LONG((long)PTC_ECODE(obj));
}
/* }}} */

/* {{{ string TC*DB::errmsg(int ecode)
 * Get the message string corresponding to to an error code.
 */
PTC_DB_METHOD(errmsg)
{
	PTC_DB_DECLARE_OBJECT(obj);
	const char *errmsg;
	long ecode;

	if (parse_parameters("l", &ecode) == FAILURE) {
		return;
	}

	errmsg = obj->functions->errmsg((int)ecode);
	if (errmsg == obj->functions->errmsg(-1)) {
		errmsg = ptc_errmsg((int)ecode);
	}

	RETURN_STRINGL((char *)errmsg, strlen(errmsg), 1);
}
/* }}} */

/* {{{ bool TC*DB::seterrmode(int emode)
 * Set the error handling mode.
 */
PTC_DB_METHOD(seterrmode)
{
	PTC_DB_DECLARE_OBJECT(obj);
	long emode;

	if (parse_parameters("l", &emode) == FAILURE) {
		return;
	}

	switch (emode) {
	case PTC_ERRMODE_SILENT:
	case PTC_ERRMODE_WARNING:
	case PTC_ERRMODE_EXCEPTION:
		obj->emode = (ptc_emode)emode;
		RETURN_TRUE;
	default:
		ptc_raise_error(obj->emode, PTC_E_PARAM TSRMLS_CC);
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ bool TC*DB::setmutex(void)
 * Set mutual exclusion control of a database object for threading.
 */
PTC_DB_METHOD(setmutex)
{
	PTC_DB_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (!obj->functions->setmutex(obj->db)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} TC*DB::setmutex */

/* {{{ bool TC*DB::open(string path, int omode)
 * Open a database file and connect a database object.
 */
PTC_DB_METHOD(open)
{
	PTC_DB_DECLARE_OBJECT(obj);
	const char *path;
	int path_len;
	long omode;

	if (parse_parameters("sl", &path, &path_len, &omode) == FAILURE) {
		return;
	}

	if (!obj->functions->open(obj->db, path, (int)omode)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}

	if (obj->functions->iterinit) {
		if (!obj->functions->iterinit(obj->db)) {
			ptc_db_raise_error(obj TSRMLS_CC);
			RETURN_FALSE;
		}

		if (obj->numeric_key) {
			obj->ikey.u64 = obj->functions->u.i.iternext(obj->db);
		} else {
			obj->ikey.str.buf = obj->functions->u.s.iternext(obj->db, &obj->ikey.str.siz);
		}
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ bool TC*DB::close(void)
 * Close a database object.
 */
PTC_DB_METHOD(close)
{
	PTC_DB_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (!obj->functions->close(obj->db)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ bool TC*DB::sync(void)
 *  Synchronize updated contents of a database object with the file and the device.
 */
PTC_DB_METHOD(sync)
{
	PTC_DB_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (!obj->functions->sync(obj->db)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ bool TC*DB::vanish(void)
 * Remove all records of a database object.
 */
PTC_DB_METHOD(vanish)
{
	PTC_DB_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (!obj->functions->vanish(obj->db)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ bool TC*DB::copy(void)
 * Copy the database file of a database object.
 */
PTC_DB_METHOD(copy)
{
	PTC_DB_DECLARE_OBJECT(obj);
	const char *path;
	int path_len;

	if (parse_parameters("s", &path, &path_len) == FAILURE) {
		return;
	}

	if (!obj->functions->copy(obj->db, path)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ string TC*DB::path(void)
 * Get the file path of a database object.
 */
PTC_DB_METHOD(path)
{
	PTC_DB_DECLARE_OBJECT(obj);
	const char *path;

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if ((path = obj->functions->path(obj->db)) == NULL) {
		RETURN_NULL();
	}

	RETURN_STRINGL((char *)path, strlen(path), 1);
}
/* }}} */

/* {{{ int TC*DB::rnum(void)
 * Get the number of records of a database object.
 */
PTC_DB_METHOD(rnum)
{
	PTC_DB_DECLARE_OBJECT(obj);
	uint64_t rnum;

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	rnum = obj->functions->rnum(obj->db);
	RETURN_UINT64(rnum);
}
/* }}} */

/* {{{ int TC*DB::fsiz(void)
 * Get the size of the database file of a database object.
 */
PTC_DB_METHOD(fsiz)
{
	PTC_DB_DECLARE_OBJECT(obj);
	uint64_t fsiz;

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	fsiz = obj->functions->fsiz(obj->db);
	RETURN_UINT64(fsiz);
}
/* }}} */

/* {{{ bool TC*DB::put*(string key, string value)
 * Store a record into a database object.
 */
PTC_LOCAL void
PTC_DB_MN_S(put_impl)(INTERNAL_FUNCTION_PARAMETERS, ptc_sput_func_t put_func, int ignore)
{
	PTC_DB_DECLARE_OBJECT(obj);
	void *kbuf, *vbuf;
	int ksiz, vsiz;

	if (FAILURE == ptc_parse_param_key_value(obj,
			&kbuf, &ksiz, obj->ktype, &vbuf, &vsiz, obj->vtype))
	{
		return;
	}

	if (!put_func(obj->db, kbuf, ksiz, vbuf, vsiz)) {
		if (PTC_ECODE(obj) != ignore) {
			ptc_db_raise_error(obj TSRMLS_CC);
		}
		RETVAL_FALSE;
	} else {
		RETVAL_TRUE;
	}
	PTC_FREE_SERIALIZED(obj->vtype, vbuf);
}
/* }}} */

/* {{{ bool TC*DB::put*(int id, string value)
 * Store a record into a database object.
 */
PTC_LOCAL void
PTC_DB_MN_I(put_impl)(INTERNAL_FUNCTION_PARAMETERS, ptc_iput_func_t put_func, int ignore)
{
	PTC_DB_DECLARE_OBJECT(obj);
	void *vbuf;
	int vsiz;

	if (FAILURE == ptc_parse_param_id_value(obj, &vbuf, &vsiz, obj->vtype)) {
		return;
	}

	if (!put_func(obj->db, obj->mkey.i64, vbuf, vsiz)) {
		if (PTC_ECODE(obj) != ignore) {
			ptc_db_raise_error(obj TSRMLS_CC);
		}
		RETVAL_FALSE;
	} else {
		RETVAL_TRUE;
	}
	PTC_FREE_SERIALIZED(obj->vtype, vbuf);
}
/* }}} */

/* {{{ bool TC*DB::put(string key, string value)
 * Store a record into a database object.
 */
PTC_DB_METHOD_S(put)
{
	PTC_DB_MN_S(put_impl)(INTERNAL_FUNCTION_PARAM_PASSTHRU,
			((ptc_db *)get_object(getThis()))->functions->u.s.put, TCESUCCESS);
}
/* }}} */

/* {{{ bool TC*DB::put(int id, string value)
 * Store a record into a database object.
 */
PTC_DB_METHOD_I(put)
{
	PTC_DB_MN_I(put_impl)(INTERNAL_FUNCTION_PARAM_PASSTHRU,
			((ptc_db *)get_object(getThis()))->functions->u.i.put, TCESUCCESS);
}
/* }}} */

/* {{{ bool TC*DB::putkeep(string key, string value)
 * Store a new record into a database object.
 */
PTC_DB_METHOD_S(putkeep)
{
	PTC_DB_MN_S(put_impl)(INTERNAL_FUNCTION_PARAM_PASSTHRU,
			((ptc_db *)get_object(getThis()))->functions->u.s.putkeep, TCEKEEP);
}
/* }}} */

/* {{{ bool TC*DB::putkeep(int id, string value)
 * Store a new record into a database object.
 */
PTC_DB_METHOD_I(putkeep)
{
	PTC_DB_MN_I(put_impl)(INTERNAL_FUNCTION_PARAM_PASSTHRU,
			((ptc_db *)get_object(getThis()))->functions->u.i.putkeep, TCEKEEP);
}
/* }}} */

/* {{{ bool TC*DB::putcat(string key, string value)
 * Concatenate a value at the end of the existing record in a database object.
 */
PTC_DB_METHOD_S(putcat)
{
	PTC_DB_MN_S(put_impl)(INTERNAL_FUNCTION_PARAM_PASSTHRU,
			((ptc_db *)get_object(getThis()))->functions->u.s.putcat, TCESUCCESS);
}
/* }}} */

/* {{{ bool TC*DB::putcat(int id, string value)
 * Concatenate a value at the end of the existing record in a database object.
 */
PTC_DB_METHOD_I(putcat)
{
	PTC_DB_MN_I(put_impl)(INTERNAL_FUNCTION_PARAM_PASSTHRU,
			((ptc_db *)get_object(getThis()))->functions->u.i.putcat, TCESUCCESS);
}
/* }}} */

/* {{{ bool TC*DB::out(string key) */
PTC_DB_METHOD_S(out)
{
	PTC_DB_DECLARE_OBJECT(obj);
	void *kbuf;
	int ksiz;

	if (FAILURE == ptc_parse_param_key(obj, &kbuf, &ksiz, obj->ktype)) {
		return;
	}

	if (!obj->functions->u.s.out(obj->db, kbuf, ksiz)) {
		if (PTC_ECODE(obj) != TCENOREC) {
			ptc_db_raise_error(obj TSRMLS_CC);
		}
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ bool TC*DB::out(int id) */
PTC_DB_METHOD_I(out)
{
	PTC_DB_DECLARE_OBJECT(obj);

	if (FAILURE == ptc_parse_param_id(obj)) {
		return;
	}

	if (!obj->functions->u.i.out(obj->db, obj->mkey.i64)) {
		if (PTC_ECODE(obj) != TCENOREC) {
			ptc_db_raise_error(obj TSRMLS_CC);
		}
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ string TC*DB::get(string key) */
PTC_DB_METHOD_S(get)
{
	PTC_DB_DECLARE_OBJECT(obj);
	void *kbuf, *vbuf;
	int ksiz, vsiz;

	if (FAILURE == ptc_parse_param_key(obj, &kbuf, &ksiz, obj->ktype)) {
		return;
	}

	if (vbuf = obj->functions->u.s.get(obj->db, kbuf, ksiz, &vsiz)) {
		if (ptc_set_retval_value(vbuf, vsiz, obj->vtype) == FAILURE) {
			ptc_raise_error(obj->emode, PTC_E_CONVERSION TSRMLS_CC);
			RETVAL_FALSE;
		}
		free(vbuf);
	} else {
		if (PTC_ECODE(obj) != TCENOREC) {
			ptc_db_raise_error(obj TSRMLS_CC);
			RETURN_FALSE;
		}
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ string TC*DB::get(int id) */
PTC_DB_METHOD_I(get)
{
	PTC_DB_DECLARE_OBJECT(obj);
	void *vbuf;
	int vsiz;

	if (FAILURE == ptc_parse_param_id(obj)) {
		return;
	}

	if (vbuf = obj->functions->u.i.get(obj->db, obj->mkey.i64, &vsiz)) {
		if (ptc_set_retval_value(vbuf, vsiz, obj->vtype) == FAILURE) {
			ptc_raise_error(obj->emode, PTC_E_CONVERSION TSRMLS_CC);
			RETVAL_FALSE;
		}
		free(vbuf);
	} else {
		if (PTC_ECODE(obj) != TCENOREC) {
			ptc_db_raise_error(obj TSRMLS_CC);
			RETURN_FALSE;
		}
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ int TC*DB::vsiz(string key)
 * Get the size of the value of a record in a database object.
 */
PTC_DB_METHOD_S(vsiz)
{
	PTC_DB_DECLARE_OBJECT(obj);
	void *kbuf;
	int ksiz;

	if (FAILURE == ptc_parse_param_key(obj, &kbuf, &ksiz, obj->ktype)) {
		return;
	}

	RETURN_LONG(obj->functions->u.s.vsiz(obj->db, kbuf, ksiz));
}
/* }}} */

/* {{{ int TC*DB::vsiz(int id)
 * Get the size of the value of a record in a database object.
 */
PTC_DB_METHOD_I(vsiz)
{
	PTC_DB_DECLARE_OBJECT(obj);

	if (FAILURE == ptc_parse_param_id(obj)) {
		return;
	}

	RETURN_LONG(obj->functions->u.i.vsiz(obj->db, obj->mkey.i64));
}
/* }}} */

/* {{{ array TC*DB::fwmkeys(string prefix[, int max])
 * Get forward matching keys in a database object.
 */
PTC_DB_METHOD(fwmkeys)
{
	PTC_DB_DECLARE_OBJECT(obj);
	zval **zpreix;
	void *pbuf;
	const void *kbuf;
	int psiz, ksiz, is_copy, i, num;
	long max = -1L;
	TCLIST *keys;

	if (FAILURE == parse_parameters("Z|l", &zpreix, &max)) {
		return;
	}

	is_copy = ptc_get_zval_key(zpreix, &pbuf, &psiz, obj->ktype);

	keys = obj->functions->fwmkeys(obj->db, pbuf, psiz, ((max < 0L) ? -1 : (int)max));
	num = tclistnum(keys);
	array_init_size(return_value, (uint)num);

	for (i = 0; i < num; i++) {
		zval *pzv;

		ALLOC_INIT_ZVAL(pzv);
		kbuf = tclistval(keys, i, &ksiz);
		if (ptc_set_zval_key(pzv, kbuf, ksiz, obj->ktype) == FAILURE) {
			ptc_raise_error(obj->emode, PTC_E_CONVERSION TSRMLS_CC);
			zval_ptr_dtor(&pzv);
			zval_dtor(return_value);
			RETVAL_FALSE;
			break;
		}
		add_next_index_zval(return_value, pzv);
	}

	tclistdel(keys);
	if (is_copy) {
		efree(pbuf);
	}
}
/* }}} */

/* {{{ bool TC*DB::addint(string key, int num)
 * Add an integer to a record in a database object.
 */
PTC_DB_METHOD_S(addint)
{
	PTC_DB_DECLARE_OBJECT(obj);
	zval **znum;
	void *kbuf, *vbuf;
	int ksiz, vsiz;

	if (FAILURE == ptc_parse_param_key_value(obj,
			&kbuf, &ksiz, obj->ktype, &vbuf, &vsiz, PTC_VALUE_IS_ZVAL))
	{
		return;
	}

	znum = (zval **)vbuf;
	convert_to_long_ex(znum);

	if (!obj->functions->u.s.addint(obj->db, kbuf, ksiz, (int)Z_LVAL_PP(znum))) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ bool TC*DB::addint(int id, string num)
 * Add an integer to a record in a database object.
 */
PTC_DB_METHOD_I(addint)
{
	PTC_DB_DECLARE_OBJECT(obj);
	zval **znum;
	void *vbuf;
	int vsiz;

	if (FAILURE == ptc_parse_param_id_value(obj,
			&vbuf, &vsiz, PTC_VALUE_IS_ZVAL))
	{
		return;
	}

	znum = (zval **)vbuf;
	convert_to_long_ex(znum);

	if (!obj->functions->u.i.addint(obj->db, obj->mkey.i64, (int)Z_LVAL_PP(znum))) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ bool TC*DB::adddouble(string key, float num)
 * Add a real number to a record in a database object.
 */
PTC_DB_METHOD_S(adddouble)
{
	PTC_DB_DECLARE_OBJECT(obj);
	zval **znum;
	void *kbuf, *vbuf;
	int ksiz, vsiz;

	if (FAILURE == ptc_parse_param_key_value(obj,
			&kbuf, &ksiz, obj->ktype, &vbuf, &vsiz, PTC_VALUE_IS_ZVAL))
	{
		return;
	}

	znum = (zval **)vbuf;
	convert_to_double_ex(znum);

	if (!obj->functions->u.s.adddouble(obj->db, kbuf, ksiz, Z_DVAL_PP(znum))) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ bool TC*DB::adddouble(int id, float num)
 * Add a real number to a record in a database object.
 */
PTC_DB_METHOD_I(adddouble)
{
	PTC_DB_DECLARE_OBJECT(obj);
	zval **znum;
	void *vbuf;
	int vsiz;

	if (FAILURE == ptc_parse_param_id_value(obj,
			&vbuf, &vsiz, PTC_VALUE_IS_ZVAL))
	{
		return;
	}

	znum = (zval **)vbuf;
	convert_to_double_ex(znum);

	if (!obj->functions->u.i.adddouble(obj->db, obj->mkey.i64, Z_DVAL_PP(znum))) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ int TC*DB::getint(string key)
 * Retrieve an integer record in a database object.
 */
PTC_DB_METHOD_S(getint)
{
	PTC_DB_DECLARE_OBJECT(obj);
	void *kbuf, *vbuf;
	int ksiz, vsiz;

	if (FAILURE == ptc_parse_param_key(obj, &kbuf, &ksiz, obj->ktype)) {
		return;
	}

	vbuf = obj->functions->u.s.get(obj->db, kbuf, ksiz, &vsiz);
	if (vbuf == NULL) {
		if (PTC_ECODE(obj) != TCENOREC) {
			ptc_db_raise_error(obj TSRMLS_CC);
			RETURN_FALSE;
		}
		RETURN_NULL();
	}

	if (vsiz != sizeof(int)) {
		ptc_raise_error(obj->emode, PTC_E_CONVERSION TSRMLS_CC);
		RETVAL_FALSE;
	} else {
		RETVAL_LONG((long)(*((int *)vbuf)));
	}
	free(vbuf);
}
/* }}} */

/* {{{ int TC*DB::getint(int id)
 * Retrieve an integer record in a database object.
 */
PTC_DB_METHOD_I(getint)
{
	PTC_DB_DECLARE_OBJECT(obj);
	void *vbuf;
	int vsiz;

	if (FAILURE == ptc_parse_param_id(obj)) {
		return;
	}

	vbuf = obj->functions->u.i.get(obj->db, obj->mkey.i64, &vsiz);
	if (vbuf == NULL) {
		if (PTC_ECODE(obj) != TCENOREC) {
			ptc_db_raise_error(obj TSRMLS_CC);
			RETURN_FALSE;
		}
		RETURN_NULL();
	}

	if (vsiz != sizeof(int)) {
		ptc_raise_error(obj->emode, PTC_E_CONVERSION TSRMLS_CC);
		RETVAL_FALSE;
	} else {
		RETVAL_LONG((long)(*((int *)vbuf)));
	}
	free(vbuf);
}
/* }}} */

/* {{{ float TC*DB::getdouble(string key)
 * Retrieve a real number record in a database object.
 */
PTC_DB_METHOD_S(getdouble)
{
	PTC_DB_DECLARE_OBJECT(obj);
	void *kbuf, *vbuf;
	int ksiz, vsiz;

	if (FAILURE == ptc_parse_param_key(obj, &kbuf, &ksiz, obj->ktype)) {
		return;
	}

	vbuf = obj->functions->u.s.get(obj->db, kbuf, ksiz, &vsiz);
	if (vbuf == NULL) {
		if (PTC_ECODE(obj) != TCENOREC) {
			ptc_db_raise_error(obj TSRMLS_CC);
			RETURN_FALSE;
		}
		RETURN_NULL();
	}

	if (vsiz != sizeof(double)) {
		ptc_raise_error(obj->emode, PTC_E_CONVERSION TSRMLS_CC);
		RETVAL_FALSE;
	} else {
		RETVAL_DOUBLE(*((double *)vbuf));
	}
	free(vbuf);
}
/* }}} */

/* {{{ float TC*DB::getdouble(int id)
 * Retrieve a real number record in a database object.
 */
PTC_DB_METHOD_I(getdouble)
{
	PTC_DB_DECLARE_OBJECT(obj);
	void *vbuf;
	int vsiz;

	if (FAILURE == ptc_parse_param_id(obj)) {
		return;
	}

	vbuf = obj->functions->u.i.get(obj->db, obj->mkey.i64, &vsiz);
	if (vbuf == NULL) {
		if (PTC_ECODE(obj) != TCENOREC) {
			ptc_db_raise_error(obj TSRMLS_CC);
			RETURN_FALSE;
		}
		RETURN_NULL();
	}

	if (vsiz != sizeof(double)) {
		ptc_raise_error(obj->emode, PTC_E_CONVERSION TSRMLS_CC);
		RETVAL_FALSE;
	} else {
		RETVAL_DOUBLE(*((double *)vbuf));
	}
	free(vbuf);
}
/* }}} */

/* {{{ string TC*DB::current(void) */
PTC_IT_METHOD(current)
{
	PTC_DB_DECLARE_OBJECT(obj);
	void *vbuf;
	int vsiz;

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (obj->numeric_key) {
		if (obj->ikey.u64 == 0) {
			RETURN_NULL();
		}
		vbuf = obj->functions->u.i.get(obj->db, (int64_t)obj->ikey.u64, &vsiz);
	} else {
		if (obj->ikey.str.buf == NULL) {
			RETURN_NULL();
		}
		vbuf = obj->functions->u.s.get(obj->db, obj->ikey.str.buf, obj->ikey.str.siz, &vsiz);
	}

	if (vbuf == NULL) {
		RETURN_NULL();
	}

	if (ptc_set_retval_value(vbuf, vsiz, obj->vtype) == FAILURE) {
		ptc_raise_error(obj->emode, PTC_E_CONVERSION TSRMLS_CC);
		RETVAL_FALSE;
	}
	free(vbuf);
}
/* }}} */

/* {{{ string TC*DB::key(void) */
PTC_IT_METHOD(key)
{
	PTC_DB_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (obj->numeric_key) {
		RETURN_UINT64(obj->ikey.u64);
	} else {
		if (obj->ikey.str.buf == NULL) {
			RETURN_NULL();
		}
		if (ptc_set_retval_key(obj->ikey.str.buf, obj->ikey.str.siz, obj->ktype) == FAILURE) {
			ptc_raise_error(obj->emode, PTC_E_CONVERSION TSRMLS_CC);
			RETURN_FALSE;
		}
	}
}
/* }}} */

/* {{{ void TC*DB::next(void) */
PTC_IT_METHOD(next)
{
	PTC_DB_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (obj->numeric_key) {
		obj->ikey.u64 = obj->functions->u.i.iternext(obj->db);
	} else {
		if (obj->ikey.str.buf) {
			free(obj->ikey.str.buf);
		}
		obj->ikey.str.buf = obj->functions->u.s.iternext(obj->db, &obj->ikey.str.siz);
	}
}
/* }}} */

/* {{{ void TC*DB::rewind(void) */
PTC_IT_METHOD(rewind)
{
	PTC_DB_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	obj->functions->iterinit(obj->db);
	if (obj->numeric_key) {
		obj->ikey.u64 = obj->functions->u.i.iternext(obj->db);
	} else {
		if (obj->ikey.str.buf) {
			free(obj->ikey.str.buf);
		}
		obj->ikey.str.buf = obj->functions->u.s.iternext(obj->db, &obj->ikey.str.siz);
	}
}
/* }}} */

/* {{{ bool TC*DB::valid(void) */
PTC_IT_METHOD(valid)
{
	PTC_DB_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (obj->numeric_key) {
		RETURN_BOOL((obj->ikey.u64 == 0) ? 0 : 1);
	} else {
		RETURN_BOOL((obj->ikey.str.buf == NULL) ? 0 : 1);
	}
}
/* }}} */


/* {{{ bool TC*DB::offsetExists(string key) */
PTC_AA_METHOD_S(offsetExists)
{
	PTC_DB_DECLARE_OBJECT(obj);
	void *kbuf;
	int ksiz;

	if (FAILURE == ptc_parse_param_key(obj, &kbuf, &ksiz, obj->ktype)) {
		return;
	}

	RETVAL_BOOL((obj->functions->u.s.vsiz(obj->db, kbuf, ksiz) == -1) ? 0 : 1);
}
/* }}} */

/* {{{ bool TC*DB::offsetExists(int id) */
PTC_AA_METHOD_I(offsetExists)
{
	PTC_DB_DECLARE_OBJECT(obj);

	if (FAILURE == ptc_parse_param_id(obj)) {
		return;
	}

	RETVAL_BOOL((obj->functions->u.i.vsiz(obj->db, obj->mkey.i64) == -1) ? 0 : 1);
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
