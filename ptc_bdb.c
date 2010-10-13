/*
 * Tokyo Cabinet PHP Bindings - The B+ tree database API of Tokyo Cabinet
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
#include <tcbdb.h>

/* {{{ macros */

#define PTC_BDB_METHOD(name) static PHP_METHOD(TCBDB, name)
#define PTC_BDB_ME(name, arg_info) PHP_ME(TCBDB, name, arg_info, ZEND_ACC_PUBLIC)

#define PTC_BDBCUR_METHOD(name) static PHP_METHOD(TCBDBCUR, name)
#define PTC_BDBCUR_ME(name, arg_info) PHP_ME(TCBDBCUR, name, arg_info, ZEND_ACC_PUBLIC)
#define PTC_BDBCUR_MALIAS(name, orig, arg_info) \
	ZEND_FENTRY(name, ZEND_MN(TCBDBCUR_##orig), arg_info, ZEND_ACC_PUBLIC)

#define PTC_BDBITER_METHOD(name) static PHP_METHOD(TCBDBITER, name)
#define PTC_BDBITER_ME(name, arg_info) PHP_ME(TCBDBITER, name, arg_info, ZEND_ACC_PUBLIC)

#define PTC_BDBCUR_DECLARE_OBJECT(obj) ptc_bdbcur *obj = (ptc_bdbcur *)get_object(getThis())

/* }}} */

/* {{{ globals */

static zend_class_entry *ce_TCBDB, *ce_TCBDBCUR, *ce_TCBDBITER;
static zend_object_handlers ptc_bdb_handlers;
static zend_object_handlers ptc_bdbcur_handlers;
static ptc_functions ptc_bdb_functions;

/* }}} */

/* {{{ internal function prototypes */

static int
class_init_TCBDBCUR(ptc_deps *deps TSRMLS_DC),
class_init_TCBDBITER(ptc_deps *deps TSRMLS_DC);

static zend_object_value
ptc_bdb_new(zend_class_entry *ce TSRMLS_DC),
ptc_bdbcur_new(zend_class_entry *ce TSRMLS_DC);

static void
ptc_bdb_free_storage(void *object TSRMLS_DC),
ptc_bdbcur_free_storage(void *object TSRMLS_DC);

static int
ptc_cmp_wrapper(const char *aptr, int asiz, const char *bptr, int bsiz, void *op);

/* }}} */

/* {{{ TCBDB method prototypes */

PTC_BDB_METHOD(__construct);
PTC_BDB_METHOD(setcmpfunc);
PTC_BDB_METHOD(tune);
PTC_BDB_METHOD(setcache);
PTC_BDB_METHOD(setxmsiz);
PTC_BDB_METHOD(putdup);
PTC_BDB_METHOD(putlist);
PTC_BDB_METHOD(outlist);
PTC_BDB_METHOD(getlist);
PTC_BDB_METHOD(vnum);
PTC_BDB_METHOD(range);
PTC_BDB_METHOD(optimize);
PTC_BDB_METHOD(tranbegin);
PTC_BDB_METHOD(trancommit);
PTC_BDB_METHOD(tranabort);
PTC_BDB_METHOD(getIterator);

/* }}} */

/* {{{ TCBDB argument informations */

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_bdb_ctor, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 0)
	ZEND_ARG_INFO(0, ktype)
	ZEND_ARG_INFO(0, vtype)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_bdb_errmsg, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, ecode)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_bdb_seterrmode, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, emode)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_bdb_open, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, path)
	ZEND_ARG_INFO(0, omode)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_bdb_copy, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, path)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_bdb_key, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_bdb_key_num, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, num)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_bdb_key_value, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_bdb_fwmkeys, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, prefix)
	ZEND_ARG_INFO(0, max)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_bdb_setcmpfunc, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, cmp)
	ZEND_ARG_INFO(0, cmpop)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_bdb_tune, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, lmemb)
	ZEND_ARG_INFO(0, nmemb)
	ZEND_ARG_INFO(0, bnum)
	ZEND_ARG_INFO(0, apow)
	ZEND_ARG_INFO(0, fpow)
	ZEND_ARG_INFO(0, opts)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_bdb_setcache, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, lcnum)
	ZEND_ARG_INFO(0, ncnum)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_bdb_setxmsiz, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, xmsiz)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_bdb_putlist, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_ARRAY_INFO(0, values, 0)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_bdb_range, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 4)
	ZEND_ARG_INFO(0, bkey)
	ZEND_ARG_INFO(0, binc)
	ZEND_ARG_INFO(0, ekey)
	ZEND_ARG_INFO(0, einc)
	ZEND_ARG_INFO(0, max)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_bdb_optimize, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, lmemb)
	ZEND_ARG_INFO(0, nmemb)
	ZEND_ARG_INFO(0, bnum)
	ZEND_ARG_INFO(0, apow)
	ZEND_ARG_INFO(0, fpow)
	ZEND_ARG_INFO(0, opts)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ TCBDBCUR method prototypes */

PTC_BDBCUR_METHOD(__construct);
PTC_BDBCUR_METHOD(first);
PTC_BDBCUR_METHOD(last);
PTC_BDBCUR_METHOD(jump);
PTC_BDBCUR_METHOD(prev);
PTC_BDBCUR_METHOD(next);
PTC_BDBCUR_METHOD(put);
PTC_BDBCUR_METHOD(out);
PTC_BDBCUR_METHOD(key);
PTC_BDBCUR_METHOD(val);
PTC_BDBCUR_METHOD(rec);

/* }}} */

/* {{{ TCBDBCUR argument informations */

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_bdbcur_ctor, ZEND_SEND_BY_VAL)
	ZEND_ARG_OBJ_INFO(0, bdb, TCBDB, 0)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_bdbcur_jump, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_bdbcur_put, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, cpmode)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ TCBDBITER method prototypes */

PTC_BDBITER_METHOD(__construct);
//PTC_BDBITER_METHOD(current); /* alias of TCBDBCUR::val */
//PTC_BDBITER_METHOD(key);     /* alias of TCBDBCUR::key */
PTC_BDBITER_METHOD(next);
PTC_BDBITER_METHOD(rewind);
PTC_BDBITER_METHOD(valid);

/* }}} */

/* {{{ TCBDBITER argument informations */

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_bdbiter_ctor, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_OBJ_INFO(0, bdb, TCBDB, 0)
	ZEND_ARG_INFO(0, reverse)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ TCBDB Methods */

/* {{{ void TCBDB::__construct([int ktype[, int vtype]])
 * Create a B+ tree database object.
 */
PTC_BDB_METHOD(__construct)
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
	obj->db = (void *)tcbdbnew();

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
/* }}} TCBDB::__construct */

/* {{{ bool TCBDB::setcmpfunc(callback cmp[, mixed cmpop])
 * Set the custom comparison function of a B+ tree database object.
 */
PTC_BDB_METHOD(setcmpfunc)
{
	PTC_DB_DECLARE_OBJECT(obj);
	ptc_compar *cmp = (ptc_compar *)obj->op;
	zval *zcmp, *zcmpop = NULL;

	if (parse_parameters("z|z", &zcmp, &zcmpop) == FAILURE) {
		return;
	}

	/* {{{ check for built-in comparison function */
	if (Z_TYPE_P(zcmp) == IS_LONG) {
		BDBCMP bcmp = NULL;
		void *bcmpop = NULL;
		int bcmptype = (int)Z_LVAL_P(zcmp);

		switch (bcmptype) {
		case PTC_BDB_CMP_LEXICAL:
			bcmp = tcbdbcmplexical;
			break;
		case PTC_BDB_CMP_DECIMAL:
			bcmp = tcbdbcmpdecimal;
			break;
		case PTC_BDB_CMP_INT32:
			bcmp = tcbdbcmpint32;
			break;
		case PTC_BDB_CMP_INT64:
			bcmp = tcbdbcmpint64;
			break;
		default:
			ptc_raise_error_ex(obj->emode, PTC_E_MISC,
					"invalid comparison function" TSRMLS_CC);
			RETURN_FALSE;
		}

		if (!tcbdbsetcmpfunc((TCBDB *)obj->db, bcmp, bcmpop)) {
			ptc_db_raise_error(obj TSRMLS_CC);
			RETURN_FALSE;
		}
		RETURN_TRUE;
	}
	/* }}} */

	/* {{{ check for user-defined comparison function */
	zval *func, *ctx;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;

	MAKE_STD_ZVAL(func);
	ZVAL_ZVAL(func, zcmp, 1, 0);

	if (zcmpop != NULL) {
		MAKE_STD_ZVAL(ctx);
		ZVAL_ZVAL(ctx, zcmpop, 1, 0);
	} else {
		ctx = NULL;
	}

	if (FAILURE ==
#if PHP_VERSION_ID >= 50300
		zend_fcall_info_init(zcmp, 0, &fci, &fcc, NULL, NULL TSRMLS_CC)
#else
		zend_fcall_info_init(zcmp, &fci, &fcc TSRMLS_CC)
#endif
	) {
		zval_ptr_dtor(&func);
		zval_ptr_dtor(&ctx);
		ptc_raise_error_ex(obj->emode, PTC_E_MISC,
				"invalid comparison function" TSRMLS_CC);
		RETURN_FALSE;
	}

	if (!tcbdbsetcmpfunc((TCBDB *)obj->db, ptc_cmp_wrapper, obj->op)) {
		zval_ptr_dtor(&func);
		zval_ptr_dtor(&ctx);
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}

	if (cmp->func) {
		zval_ptr_dtor(&cmp->func);
	}
	if (cmp->ctx) {
		zval_ptr_dtor(&cmp->ctx);
	}

	cmp->func = func;
	cmp->ctx = ctx;
	cmp->fci = fci;
	cmp->fcc = fcc;
	/* }}} */

	RETURN_TRUE;
}
/* }}} TCBDB::setcmpfunc */

/* {{{ bool TCBDB::tune(int lmemb, int nmemb, int bnum, int apow, int fpow, int opts)
 * Set the tuning parameters of a B+ tree database object.
 */
PTC_BDB_METHOD(tune)
{
	PTC_DB_DECLARE_OBJECT(obj);
	long llmemb, lnmemb, lbnum, lapow, lfpow, lopts;
	int32_t lmemb, nmemb;
	int64_t bnum;
	int8_t apow, fpow;
	uint8_t opts;

	if (FAILURE == parse_parameters("llllll",
			&llmemb, &lnmemb, &lbnum, &lapow, &lfpow, &lopts))
	{
		return;
	}

	lmemb = (llmemb < 0L) ? -1 : (int32_t)(llmemb & 0x7FFFFFFFL);
	nmemb = (lnmemb < 0L) ? -1 : (int32_t)(lnmemb & 0x7FFFFFFFL);
	bnum = (int64_t)lbnum;
	apow = (lapow < 0L) ? -1 : (int8_t)(lapow & 0x7FL);
	fpow = (lfpow < 0L) ? -1 : (int8_t)(lfpow & 0x7FL);
	opts = (lopts < 0L) ? 0 : (uint8_t)(lopts & 0xFFL);

	if (!tcbdbtune((TCBDB *)obj->db, lmemb, nmemb, bnum, apow, fpow, opts)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} TCBDB::tune */

/* {{{ bool TCBDB::setcache(int lcnum, int ncnum)
 * Set the caching parameters of a B+ tree database object.
 */
PTC_BDB_METHOD(setcache)
{
	PTC_DB_DECLARE_OBJECT(obj);
	long llcnum, lncnum;
	int32_t lcnum, ncnum;

	if (parse_parameters("ll", &llcnum, &lncnum) == FAILURE) {
		return;
	}

	lcnum = (llcnum < 0L) ? -1 : (int32_t)(llcnum & 0x7FFFFFFFL);
	ncnum = (lncnum < 0L) ? -1 : (int32_t)(lncnum & 0x7FFFFFFFL);

	if (!tcbdbsetcache((TCBDB *)obj->db, lcnum, ncnum)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} TCBDB::setcache */

/* {{{ bool TCBDB::setxmsiz(int xmsiz)
 * Set the size of the extra mapped memory of a B+ tree database object.
 */
PTC_BDB_METHOD(setxmsiz)
{
	PTC_DB_DECLARE_OBJECT(obj);
	long lxmsiz;
	int64_t xmsiz;

	if (parse_parameters("l", &lxmsiz) == FAILURE) {
		return;
	}

	xmsiz = (int64_t)lxmsiz;

	if (!tcbdbsetxmsiz((TCBDB *)obj->db, xmsiz)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} TCBDB::setxmsiz */

/* {{{ bool TCBDB::putdup(string key, string value)
 * Store a record into a B+ tree database object with allowing duplication of keys.
 */
PTC_BDB_METHOD(putdup)
{
	PTC_DB_MN_S(put_impl)(INTERNAL_FUNCTION_PARAM_PASSTHRU,
		(ptc_sput_func_t)tcbdbputdup, TCESUCCESS);
}
/* }}} TCBDB::putdup */

/* {{{ bool TCBDB::putlist(string key, array values)
 * Store a record into a B+ tree database object with allowing duplication of keys.
 */
PTC_BDB_METHOD(putlist)
{
	PTC_DB_DECLARE_OBJECT(obj);
	TCBDB *bdb = (TCBDB *)obj->db;
	void *kbuf;
	int ksiz;
	HashTable *vals;
	int vnum;
	zval **entry;
	HashPosition pos;

	if (ptc_parse_param_key_value(obj, &kbuf, &ksiz, obj->ktype,
			(void **)&vals, &vnum, PTC_VALUE_IS_HASHTABLE) == FAILURE)
	{
		return;
	}

	zend_hash_internal_pointer_reset_ex(vals, &pos);

	while (zend_hash_get_current_data_ex(vals, (void **)&entry, &pos) == SUCCESS) {
		void *vbuf;
		int vsiz, is_copy;

		is_copy = ptc_get_zval_value(entry, &vbuf, &vsiz, obj->vtype);
		if (is_copy == -1) {
			RETURN_FALSE;
		}

		if (!tcbdbputdup(bdb, kbuf, ksiz, vbuf, vsiz)) {
			if (is_copy) {
				efree(vbuf);
			}
			ptc_db_raise_error(obj TSRMLS_CC);
			RETURN_FALSE;
		}

		if (is_copy) {
			efree(vbuf);
		}
		zend_hash_move_forward_ex(vals, &pos);
	}

	RETURN_TRUE;
}
/* }}} TCBDB::putlist */

/* {{{ bool TCBDB::outlist(string key)
 * Remove records of a B+ tree database object.
 */
PTC_BDB_METHOD(outlist)
{
	PTC_DB_DECLARE_OBJECT(obj);
	TCBDB *bdb = (TCBDB *)obj->db;
	void *kbuf;
	int ksiz;

	if (ptc_parse_param_key(obj, &kbuf, &ksiz, obj->ktype) == FAILURE) {
		return;
	}

	if (!tcbdbout3(bdb, kbuf, ksiz)) {
		if (tcbdbecode(bdb) != TCENOREC) {
			ptc_db_raise_error(obj TSRMLS_CC);
		}
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} TCBDB::outlist */

/* {{{ array TCBDB::getlist(string key)
 * Retrieve records in a B+ tree database object.
 */
PTC_BDB_METHOD(getlist)
{
	PTC_DB_DECLARE_OBJECT(obj);
	TCBDB *bdb = (TCBDB *)obj->db;
	void *kbuf;
	int ksiz;
	TCLIST *values;

	if (ptc_parse_param_key(obj, &kbuf, &ksiz, obj->ktype) == FAILURE) {
		return;
	}

	if ((values = tcbdbget4(bdb, kbuf, ksiz)) == NULL) {
		if (tcbdbecode(bdb) != TCENOREC) {
			ptc_db_raise_error(obj TSRMLS_CC);
			RETVAL_FALSE;
		} else {
			RETVAL_NULL();
		}
	} else {
		int i, num;

		num = tclistnum(values);
		array_init_size(return_value, (uint)num);

		for (i = 0; i < num; i++) {
			const void *vbuf;
			int vsiz;
			zval *pzv;

			ALLOC_INIT_ZVAL(pzv);
			vbuf = tclistval(values, i, &vsiz);
			if (ptc_set_zval_value(pzv, vbuf, vsiz, obj->vtype) == FAILURE) {
				ptc_raise_error(obj->emode, PTC_E_CONVERSION TSRMLS_CC);
				zval_ptr_dtor(&pzv);
				zval_dtor(return_value);
				RETVAL_FALSE;
				break;
			}
			add_next_index_zval(return_value, pzv);
		}

		tclistdel(values);
	}
}
/* }}} TCBDB::getlist */

/* {{{ int TCBDB::vnum(string key)
 * Get the number of records corresponding a key in a B+ tree database object.
 */
PTC_BDB_METHOD(vnum)
{
	PTC_DB_DECLARE_OBJECT(obj);
	void *kbuf;
	int ksiz;

	if (ptc_parse_param_key(obj, &kbuf, &ksiz, obj->ktype) == FAILURE) {
		return;
	}

	RETURN_LONG(tcbdbvnum((TCBDB *)obj->db, kbuf, ksiz));
}
/* }}} TCBDB::vnum */

/* {{{ array TCBDB::range(string bkey, bool binc, string ekey, bool einc[, int max])
 * Get keys of ranged records in a B+ tree database object.
 */
PTC_BDB_METHOD(range)
{
	PTC_DB_DECLARE_OBJECT(obj);
	zval **zbkbuf, **zekbuf;
	void *bkbuf = NULL, *ekbuf = NULL;
	const void *kbuf;
	int bksiz = 0, eksiz = 0, biscopy = 0, eiscopy = 0;
	int ksiz, i, num;
	zend_bool binc, einc;
	long max = -1L;
	TCLIST *keys;

	if (parse_parameters("ZbZb|l", &zbkbuf, &binc, &zekbuf, &einc, &max) == FAILURE) {
		return;
	}

	if (Z_TYPE_PP(zbkbuf) != IS_NULL) {
		biscopy = ptc_get_zval_key(zbkbuf, &bkbuf, &bksiz, obj->ktype);
	}

	if (Z_TYPE_PP(zekbuf) != IS_NULL) {
		eiscopy = ptc_get_zval_key(zekbuf, &ekbuf, &eksiz, obj->ktype);
	}

	keys = tcbdbrange((TCBDB *)obj->db, bkbuf, bksiz, (bool)binc,
			ekbuf, eksiz, (bool)einc, ((max < 0L) ? -1 : (int)max));
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
	if (biscopy) {
		efree(bkbuf);
	}
	if (eiscopy) {
		efree(ekbuf);
	}
}
/* }}} TCBDB::range */

/* {{{ bool TCBDB::optimize(int lmemb, int nmemb, int bnum, int apow, int fpow, int opts)
 * Optimize the file of a B+ tree database object.
 */
PTC_BDB_METHOD(optimize)
{
	PTC_DB_DECLARE_OBJECT(obj);
	long llmemb, lnmemb, lbnum, lapow, lfpow, lopts;
	int32_t lmemb, nmemb;
	int64_t bnum;
	int8_t apow, fpow;
	uint8_t opts;

	if (FAILURE == parse_parameters("llllll",
			&llmemb, &lnmemb, &lbnum, &lapow, &lfpow, &lopts))
	{
		return;
	}

	lmemb = (llmemb < 0L) ? -1 : (int32_t)(llmemb & 0x7FFFFFFFL);
	nmemb = (lnmemb < 0L) ? -1 : (int32_t)(lnmemb & 0x7FFFFFFFL);
	bnum = (int64_t)lbnum;
	apow = (lapow < 0L) ? -1 : (int8_t)(lapow & 0x7FL);
	fpow = (lfpow < 0L) ? -1 : (int8_t)(lfpow & 0x7FL);
	opts = (lopts < 0L) ? UINT8_MAX : (uint8_t)(lopts & 0xFFL);

	if (!tcbdboptimize((TCBDB *)obj->db, lmemb, nmemb, bnum, apow, fpow, opts)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} TCBDB::optimize */

/* {{{ bool TCBDB::tranbegin(void)
 * Begin the transaction of a B+ tree database object.
 */
PTC_BDB_METHOD(tranbegin)
{
	PTC_DB_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (!tcbdbtranbegin((TCBDB *)obj->db)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} TCBDB::tranbegin */

/* {{{ bool TCBDB::trancommit(void)
 * Commit the transaction of a B+ tree database object.
 */
PTC_BDB_METHOD(trancommit)
{
	PTC_DB_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (!tcbdbtrancommit((TCBDB *)obj->db)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} TCBDB::trancommit */

/* {{{ bool TCBDB::tranabort(void)
 * Abort the transaction of a B+ tree database object.
 */
PTC_BDB_METHOD(tranabort)
{
	PTC_DB_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (!tcbdbtranabort((TCBDB *)obj->db)) {
		ptc_db_raise_error(obj TSRMLS_CC);
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} TCBDB::tranabort */

/* {{{ TCBDBITER TCBDB::getIterator(void)
 * IteratorAggregate implementation.
 */
PTC_BDB_METHOD(getIterator)
{
	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	object_init_ex(return_value, ce_TCBDBITER);
	zend_call_method_with_1_params(&return_value, ce_TCBDBITER,
			&ce_TCBDBITER->constructor, "__construct", NULL, getThis());
}
/* }}} TCBDB::getIterator */

/* }}} TCBDB Methods */

/* {{{ TCBDBCUR Methods */

/* {{{ void TCBDBCUR::__construct(TCBDB bdb)
 * Create a cursor object.
 */
PTC_BDBCUR_METHOD(__construct)
{
	PTC_BDBCUR_DECLARE_OBJECT(obj);
	zval *zbdb;

	if (parse_parameters("O", &zbdb, ce_TCBDB) == FAILURE) {
		return;
	}

	if (obj->cur != NULL) {
		ptc_raise_error(obj->p->emode, PTC_E_INVALID TSRMLS_CC);
		return;
	}

	Z_ADDREF_P(zbdb);
	obj->z = zbdb;
	obj->p = (ptc_db *)get_object(zbdb);
	obj->cur = tcbdbcurnew((TCBDB *)obj->p->db);
	obj->valid = tcbdbcurfirst(obj->cur);
}
/* }}} TCBDBCUR::__construct */

/* {{{ bool TCBDBCUR::first(void)
 * Move a cursor object to the first record.
 */
PTC_BDBCUR_METHOD(first)
{
	PTC_BDBCUR_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	RETURN_BOOL(tcbdbcurfirst(obj->cur));
}
/* }}} TCBDBCUR::first */

/* {{{ bool TCBDBCUR::last(void)
 * Move a cursor object to the last record.
 */
PTC_BDBCUR_METHOD(last)
{
	PTC_BDBCUR_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	RETURN_BOOL(tcbdbcurlast(obj->cur));
}
/* }}} TCBDBCUR::last */

/* {{{ bool TCBDBCUR::jump(string key)
 * Move a cursor object to the front of records corresponding a key.
 */
PTC_BDBCUR_METHOD(jump)
{
	PTC_BDBCUR_DECLARE_OBJECT(obj);
	void *kbuf;
	int ksiz;

	if (ptc_parse_param_key(obj->p, &kbuf, &ksiz, obj->p->ktype) == FAILURE) {
		return;
	}

	RETVAL_BOOL(tcbdbcurjump(obj->cur, kbuf, ksiz));
}
/* }}} TCBDBCUR::jump */

/* {{{ bool TCBDBCUR::prev(void)
 * Move a cursor object to the previous record.
 */
PTC_BDBCUR_METHOD(prev)
{
	PTC_BDBCUR_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	RETURN_BOOL(tcbdbcurprev(obj->cur));
}
/* }}} TCBDBCUR::prev */

/* {{{ bool TCBDBCUR::next(void)
 * Move a cursor object to the next record.
 */
PTC_BDBCUR_METHOD(next)
{
	PTC_BDBCUR_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	RETURN_BOOL(tcbdbcurnext(obj->cur));
}
/* }}} TCBDBCUR::next */

/* {{{ bool TCBDBCUR::put(string value, int cpmode)
 * Insert a record around a cursor object.
 */
PTC_BDBCUR_METHOD(put)
{
	PTC_BDBCUR_DECLARE_OBJECT(obj);
	void *vbuf, *vptr;
	int vsiz, vlen;
	long cpmode;

	if (obj->p->vtype == PTC_VALUE_IS_SERIALIZED) {
		if (parse_parameters("Zl", &vptr, &cpmode) == FAILURE) {
			return;
		}
	} else {
		if (parse_parameters("sl", &vptr, &vlen, &cpmode) == FAILURE) {
			return;
		}
	}

	switch (obj->p->vtype) {
	case PTC_VALUE_IS_SERIALIZED: {
		php_serialize_data_t var_hash;
		smart_str buf = {0};
		zval **data = (zval **)vptr;

		PHP_VAR_SERIALIZE_INIT(var_hash);
		php_var_serialize(&buf, data, &var_hash TSRMLS_CC);
		PHP_VAR_SERIALIZE_DESTROY(var_hash);

		if (buf.c == NULL) {
			raise_error(E_WARNING, "unable to serialize value");
			RETURN_FALSE;
		}
		vbuf = buf.c;
		vsiz = buf.len;
		break;
	  }

	default:
		vbuf = vptr;
		vsiz = vlen;
	}

	if (!tcbdbcurput(obj->cur, vbuf, vsiz, (int)cpmode)) {
		ptc_db_raise_error(obj->p TSRMLS_CC);
		RETVAL_FALSE;
	} else {
		RETVAL_TRUE;
	}

	PTC_FREE_SERIALIZED(obj->p->vtype, vbuf);
}
/* }}} TCBDBCUR::put */

/* {{{ bool TCBDBCUR::out(void)
 * Delete the record where a cursor object is.
 */
PTC_BDBCUR_METHOD(out)
{
	PTC_BDBCUR_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if (!tcbdbcurout(obj->cur)) {
		if (tcbdbecode((TCBDB *)obj->p->db) != TCENOREC) {
			ptc_db_raise_error(obj->p TSRMLS_CC);
		}
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} TCBDBCUR::out */

/* {{{ string TCBDBCUR::key(void)
 * Get the key of the record where the cursor object is.
 */
PTC_BDBCUR_METHOD(key)
{
	PTC_BDBCUR_DECLARE_OBJECT(obj);
	void *kbuf;
	int ksiz;

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if ((kbuf = tcbdbcurkey(obj->cur, &ksiz)) == NULL) {
		RETURN_NULL();
	}

	if (ptc_set_retval_key(kbuf, ksiz, obj->p->ktype) == FAILURE) {
		ptc_raise_error(obj->p->emode, PTC_E_CONVERSION TSRMLS_CC);
		RETVAL_FALSE;
	}
	free(kbuf);
}
/* }}} TCBDBCUR::key */

/* {{{ string TCBDBCUR::val(void)
 * Get the value of the record where the cursor object is.
 */
PTC_BDBCUR_METHOD(val)
{
	PTC_BDBCUR_DECLARE_OBJECT(obj);
	void *vbuf;
	int vsiz;

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	if ((vbuf = tcbdbcurval(obj->cur, &vsiz)) == NULL) {
		RETURN_NULL();
	}

	if (ptc_set_retval_value(vbuf, vsiz, obj->p->vtype) == FAILURE) {
		ptc_raise_error(obj->p->emode, PTC_E_CONVERSION TSRMLS_CC);
		RETVAL_FALSE;
	}
	free(vbuf);
}
/* }}} TCBDBCUR::val */

/* {{{ array TCBDBCUR::rec(void)
 * Get the key and the value of the record where the cursor object is.
 */
PTC_BDBCUR_METHOD(rec)
{
	PTC_BDBCUR_DECLARE_OBJECT(obj);
	TCXSTR *kxstr, *vxstr;

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	kxstr = tcxstrnew();
	vxstr = tcxstrnew();
	if (!tcbdbcurrec(obj->cur, kxstr, vxstr)) {
		RETVAL_NULL();
	} else {
		zval *kzv, *vzv;

		ALLOC_INIT_ZVAL(kzv);
		ALLOC_INIT_ZVAL(vzv);

		if (ptc_set_zval_key(kzv, tcxstrptr(kxstr),
				tcxstrsize(kxstr), obj->p->ktype) == FAILURE ||
			ptc_set_zval_value(vzv, tcxstrptr(vxstr),
				tcxstrsize(vxstr), obj->p->vtype) == FAILURE)
		{
			ptc_raise_error(obj->p->emode, PTC_E_CONVERSION TSRMLS_CC);
			zval_ptr_dtor(&kzv);
			zval_ptr_dtor(&vzv);
			RETVAL_FALSE;
		} else {
			array_init(return_value);
			add_next_index_zval(return_value, kzv);
			add_next_index_zval(return_value, vzv);
		}
	}
	tcxstrdel(kxstr);
	tcxstrdel(vxstr);
}
/* }}} TCBDBCUR::rec */

/* }}} TCBDBCUR Methods */

/* {{{ TCBDBITER Methods */

/* {{{ void TCBDBITER::__construct(TCBDB bdb)
 * Create an iterator object.
 */
PTC_BDBITER_METHOD(__construct)
{
	PTC_BDBCUR_DECLARE_OBJECT(obj);
	zval *zbdb;
	zend_bool reverse = 0;

	if (parse_parameters("O|b", &zbdb, ce_TCBDB, &reverse) == FAILURE) {
		return;
	}

	if (obj->cur != NULL) {
		ptc_raise_error(obj->p->emode, PTC_E_INVALID TSRMLS_CC);
		return;
	}

	Z_ADDREF_P(zbdb);
	obj->z = zbdb;
	obj->p = (ptc_db *)get_object(zbdb);
	obj->cur = tcbdbcurnew((TCBDB *)obj->p->db);
	if (reverse) {
		obj->next = tcbdbcurprev;
		obj->rewind = tcbdbcurlast;
	} else {
		obj->next = tcbdbcurnext;
		obj->rewind = tcbdbcurfirst;
	}
	obj->valid = obj->rewind(obj->cur);
}
/* }}} TCBDBITER::__construct */

/* {{{ void TCBDBITER::next(void)
 * Iterator implementation.
 */
PTC_BDBITER_METHOD(next)
{
	PTC_BDBCUR_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	obj->valid = obj->next(obj->cur);
}
/* }}} TCBDBITER::next */

/* {{{ void TCBDBITER::rewind(void)
 * Iterator implementation.
 */
PTC_BDBITER_METHOD(rewind)
{
	PTC_BDBCUR_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	obj->valid = obj->rewind(obj->cur);
}
/* }}} TCBDBITER::rewind */

/* {{{ bool TCBDBITER::valid(void)
 * Iterator implementation.
 */
PTC_BDBITER_METHOD(valid)
{
	PTC_BDBCUR_DECLARE_OBJECT(obj);

	if (ZEND_NUM_ARGS() != 0) {
		WRONG_PARAM_COUNT;
	}

	RETURN_BOOL(obj->valid);
}
/* }}} TCBDBITER::valid */

/* }}} TCBDBITER Methods */

/* {{{ class_init_TCBDB() */
PTC_LOCAL int
class_init_TCBDB(ptc_deps *deps TSRMLS_DC)
{
	zend_class_entry ce;

	/* {{{ Class registration */

	if (ptc_class_exists("tcbdb")) {
		zend_error(E_CORE_WARNING, "Cannot redeclare class TCBDB");
		return FAILURE;
	}

	(void)memcpy(&ptc_bdb_handlers,
			zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	ptc_bdb_handlers.clone_obj = NULL;

	(void)memset(&ptc_bdb_functions, 0 , sizeof(ptc_functions));
	ptc_bdb_functions.dbnew     = (ptc_new_func_t)  tcbdbnew;
	ptc_bdb_functions.del       = (ptc_del_func_t)  tcbdbdel;
	ptc_bdb_functions.errmsg    = (ptc_errmsg_func_t)   tcbdberrmsg;
	ptc_bdb_functions.ecode     = (ptc_ecode_func_t)    tcbdbecode;
	ptc_bdb_functions.setmutex  = (ptc_setmutex_func_t) tcbdbsetmutex;
	ptc_bdb_functions.open      = (ptc_open_func_t)     tcbdbopen;
	ptc_bdb_functions.close     = (ptc_close_func_t)    tcbdbclose;
	ptc_bdb_functions.sync      = (ptc_sync_func_t)     tcbdbsync;
	ptc_bdb_functions.vanish    = (ptc_vanish_func_t)   tcbdbvanish;
	ptc_bdb_functions.copy      = (ptc_copy_func_t) tcbdbcopy;
	ptc_bdb_functions.path      = (ptc_path_func_t) tcbdbpath;
	ptc_bdb_functions.rnum      = (ptc_rnum_func_t) tcbdbrnum;
	ptc_bdb_functions.fsiz      = (ptc_fsiz_func_t) tcbdbfsiz;
	//ptc_bdb_functions.iterinit  = (ptc_iterinit_func_t) tcbdbiterinit;
	ptc_bdb_functions.fwmkeys   = (ptc_fwmkeys_func_t)  tcbdbfwmkeys;
	ptc_bdb_functions.u.s.put       = (ptc_sput_func_t) tcbdbput;
	ptc_bdb_functions.u.s.putkeep   = (ptc_sput_func_t) tcbdbputkeep;
	ptc_bdb_functions.u.s.putcat    = (ptc_sput_func_t) tcbdbputcat;
	ptc_bdb_functions.u.s.out   = (ptc_sout_func_t) tcbdbout;
	ptc_bdb_functions.u.s.get   = (ptc_sget_func_t) tcbdbget;
	ptc_bdb_functions.u.s.vsiz  = (ptc_svsiz_func_t)    tcbdbvsiz;
	ptc_bdb_functions.u.s.addint    = (ptc_saddint_func_t)  tcbdbaddint;
	ptc_bdb_functions.u.s.adddouble = (ptc_sadddouble_func_t)   tcbdbadddouble;
	//ptc_bdb_functions.u.s.iternext  = (ptc_siternext_func_t)    tcbdbiternext;

	zend_function_entry TCBDB_methods[] = {
		PHP_ME(TCBDB, __construct, ptc_arg_bdb_ctor, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PTC_DB_ME(ecode,        NULL)
		PTC_DB_ME(errmsg,       ptc_arg_bdb_errmsg)
		PTC_DB_ME(seterrmode,   ptc_arg_bdb_seterrmode)
		PTC_DB_ME(setmutex,     NULL)
		PTC_BDB_ME(setcmpfunc,  ptc_arg_bdb_setcmpfunc)
		PTC_BDB_ME(tune,        ptc_arg_bdb_tune)
		PTC_BDB_ME(setcache,    ptc_arg_bdb_setcache)
		PTC_BDB_ME(setxmsiz,    ptc_arg_bdb_setxmsiz)
		PTC_DB_ME(open,         ptc_arg_bdb_open)
		PTC_DB_ME(close,        NULL)
		PTC_DB_ME_S(put,        ptc_arg_bdb_key_value)
		PTC_DB_ME_S(putkeep,    ptc_arg_bdb_key_value)
		PTC_DB_ME_S(putcat,     ptc_arg_bdb_key_value)
		PTC_BDB_ME(putdup,      ptc_arg_bdb_key_value)
		PTC_BDB_ME(putlist,     ptc_arg_bdb_putlist)
		PTC_DB_ME_S(out,        ptc_arg_bdb_key)
		PTC_BDB_ME(outlist,     ptc_arg_bdb_key)
		PTC_DB_ME_S(get,        ptc_arg_bdb_key)
		PTC_BDB_ME(getlist,     ptc_arg_bdb_key)
		PTC_BDB_ME(vnum,        ptc_arg_bdb_key)
		PTC_DB_ME_S(vsiz,       ptc_arg_bdb_key)
		PTC_BDB_ME(range,       ptc_arg_bdb_range)
		PTC_DB_ME(fwmkeys,      ptc_arg_bdb_fwmkeys)
		PTC_DB_ME_S(addint,     ptc_arg_bdb_key_num)
		PTC_DB_ME_S(adddouble,  ptc_arg_bdb_key_num)
		PTC_DB_ME_S(getint,     ptc_arg_bdb_key)
		PTC_DB_ME_S(getdouble,  ptc_arg_bdb_key)
		PTC_DB_ME(sync,         NULL)
		PTC_BDB_ME(optimize,    ptc_arg_bdb_optimize)
		PTC_DB_ME(vanish,       NULL)
		PTC_DB_ME(copy,         ptc_arg_bdb_copy)
		PTC_BDB_ME(tranbegin,   NULL)
		PTC_BDB_ME(trancommit,  NULL)
		PTC_BDB_ME(tranabort,   NULL)
		PTC_DB_ME(path,         NULL)
		PTC_DB_ME(rnum,         NULL)
		PTC_DB_ME(fsiz,         NULL)
		/* IteratorAggregate implementation */
		PTC_BDB_ME(getIterator, NULL)
		/* ArrayAccess implementations */
		PTC_AA_ME_S(offsetExists,   ptc_arg_bdb_key)
		PTC_DB_MALIAS_S(offsetGet,   get, ptc_arg_bdb_key)
		PTC_DB_MALIAS_S(offsetSet,   put, ptc_arg_bdb_key_value)
		PTC_DB_MALIAS_S(offsetUnset, out, ptc_arg_bdb_key)
		/* Countable implementation */
		PTC_DB_MALIAS(count, rnum, NULL)
		{ NULL, NULL, NULL }
	};

	INIT_CLASS_ENTRY(ce, "TCBDB", TCBDB_methods);
	ce_TCBDB = zend_register_internal_class(&ce TSRMLS_CC);
	if (ce_TCBDB == NULL) {
		return FAILURE;
	}
	ce_TCBDB->create_object = ptc_bdb_new;
	ce_TCBDB->ce_flags |= ZEND_ACC_FINAL_CLASS;

	zend_class_implements(ce_TCBDB TSRMLS_CC, 4, deps->base,
			deps->iteratoraggregate, deps->arrayaccess, deps->countable);

	/* }}} */

	/* {{{ Constant registration */
#define ptc_bdb_register_constant(name, value) \
	zend_declare_class_constant_long(ce_TCBDB, name, sizeof(name)-1, value TSRMLS_CC)

	ptc_bdb_register_constant("OWRITER",  BDBOWRITER);
	ptc_bdb_register_constant("OREADER",  BDBOREADER);
	ptc_bdb_register_constant("OCREAT",   BDBOCREAT);
	ptc_bdb_register_constant("OTRUNC",   BDBOTRUNC);
	ptc_bdb_register_constant("ONOLCK",   BDBONOLCK);
	ptc_bdb_register_constant("OLCKNB",   BDBOLCKNB);

	ptc_bdb_register_constant("TLARGE",   BDBTLARGE);
	ptc_bdb_register_constant("TDEFLATE", BDBTDEFLATE);
	ptc_bdb_register_constant("TBZIP",    BDBTBZIP);
	ptc_bdb_register_constant("TTCBS",    BDBTTCBS);

	ptc_bdb_register_constant("CPCURRENT",BDBCPCURRENT);
	ptc_bdb_register_constant("CPBEFORE", BDBCPBEFORE);
	ptc_bdb_register_constant("CPAFTER",  BDBCPAFTER);

	ptc_bdb_register_constant("CMPLEXICAL",   PTC_BDB_CMP_LEXICAL);
	ptc_bdb_register_constant("CMPDECIMAL",   PTC_BDB_CMP_DECIMAL);
	ptc_bdb_register_constant("CMPINT32",     PTC_BDB_CMP_INT32);
	ptc_bdb_register_constant("CMPINT64",     PTC_BDB_CMP_INT64);

	ptc_bdb_register_constant("KTBINARY",     PTC_IS_BINARY);
	ptc_bdb_register_constant("KTSTRING",     PTC_IS_BINARY);
	ptc_bdb_register_constant("KTINT32",      PTC_IS_INT32);
	ptc_bdb_register_constant("KTINT64",      PTC_IS_INT64);

	ptc_bdb_register_constant("VTBINARY",     PTC_IS_BINARY);
	ptc_bdb_register_constant("VTSTRING",     PTC_IS_BINARY);
	ptc_bdb_register_constant("VTSERIALIZED", PTC_IS_SERIALIZED);

#undef ptc_bdb_register_constant
	/* }}} */

	if (FAILURE == class_init_TCBDBCUR(deps TSRMLS_CC) ||
		FAILURE == class_init_TCBDBITER(deps TSRMLS_CC))
	{
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ class_init_TCBDBCUR() */
static int
class_init_TCBDBCUR(ptc_deps *deps TSRMLS_DC)
{
	zend_class_entry ce;

	/* {{{ Class registration */

	if (ptc_class_exists("tcbdbcur")) {
		zend_error(E_CORE_WARNING, "Cannot redeclare class TCBDBCUR");
		return FAILURE;
	}

	(void)memcpy(&ptc_bdbcur_handlers,
			zend_get_std_object_handlers(), sizeof(zend_object_handlers));
	ptc_bdbcur_handlers.clone_obj = NULL;

	zend_function_entry TCBDBCUR_methods[] = {
		PHP_ME(TCBDBCUR, __construct, ptc_arg_bdbcur_ctor, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		PTC_BDBCUR_ME(first,    NULL)
		PTC_BDBCUR_ME(last,     NULL)
		PTC_BDBCUR_ME(jump,     ptc_arg_bdbcur_jump)
		PTC_BDBCUR_ME(prev,     NULL)
		PTC_BDBCUR_ME(next,     NULL)
		PTC_BDBCUR_ME(put,      ptc_arg_bdbcur_put)
		PTC_BDBCUR_ME(out,      NULL)
		PTC_BDBCUR_ME(key,      NULL)
		PTC_BDBCUR_ME(val,      NULL)
		PTC_BDBCUR_ME(rec,      NULL)
		{ NULL, NULL, NULL }
	};

	INIT_CLASS_ENTRY(ce, "TCBDBCUR", TCBDBCUR_methods);
	ce_TCBDBCUR = zend_register_internal_class(&ce TSRMLS_CC);
	if (ce_TCBDBCUR == NULL) {
		return FAILURE;
	}
	ce_TCBDBCUR->create_object = ptc_bdbcur_new;
	ce_TCBDBCUR->ce_flags |= ZEND_ACC_FINAL_CLASS;

	/* }}} */

	/* {{{ Property registration */

	(void)zend_declare_property_null(ce_TCBDBCUR, "bdb", 3, ZEND_ACC_PRIVATE TSRMLS_CC);

	/* }}} */

	return SUCCESS;
}
/* }}} */

/* {{{ class_init_TCBDBITER() */
static int
class_init_TCBDBITER(ptc_deps *deps TSRMLS_DC)
{
	zend_class_entry ce;

	/* {{{ Class registration */

	if (ptc_class_exists("tcbdbiter")) {
		zend_error(E_CORE_WARNING, "Cannot redeclare class TCBDBITER");
		return FAILURE;
	}

	zend_function_entry TCBDBITER_methods[] = {
		PHP_ME(TCBDBITER, __construct, ptc_arg_bdbiter_ctor, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
		/* Iterator implementations */
		PTC_BDBCUR_MALIAS(current, val, NULL)
		PTC_BDBCUR_MALIAS(key,     key, NULL)
		PTC_BDBITER_ME(next,    NULL)
		PTC_BDBITER_ME(rewind,  NULL)
		PTC_BDBITER_ME(valid,   NULL)
		{ NULL, NULL, NULL }
	};

	INIT_CLASS_ENTRY(ce, "TCBDBITER", TCBDBITER_methods);
	ce_TCBDBITER = zend_register_internal_class(&ce TSRMLS_CC);
	if (ce_TCBDBITER == NULL) {
		return FAILURE;
	}
	ce_TCBDBITER->create_object = ptc_bdbcur_new;
	ce_TCBDBITER->ce_flags |= ZEND_ACC_FINAL_CLASS;

	zend_class_implements(ce_TCBDBITER TSRMLS_CC, 1, deps->iterator);

	/* }}} */

	/* {{{ Property registration */

	(void)zend_declare_property_null(ce_TCBDBITER, "bdb", 3, ZEND_ACC_PRIVATE TSRMLS_CC);

	/* }}} */

	return SUCCESS;
}
/* }}} */

/* {{{ ptc_bdb_new() */
static zend_object_value
ptc_bdb_new(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value retval;
	ptc_db *obj;
	ptc_compar *cmp;

	obj = (ptc_db *)ecalloc(1, sizeof(ptc_db));
	obj->type = PTC_BDB;
	obj->numeric_key = 0;
	obj->emode = PTC_ERRMODE_EXCEPTION;
	obj->functions = &ptc_bdb_functions;

	cmp = (ptc_compar *)ecalloc(1, sizeof(ptc_compar));
	PTC_OP_TSRMLS_SET(cmp);
	obj->op = cmp;

	zend_object_std_init(&obj->std, ce TSRMLS_CC);\
	object_properties_init(&obj->std, ce);

	retval.handle = zend_objects_store_put(obj,
			(zend_objects_store_dtor_t)zend_objects_destroy_object,
			(zend_objects_free_object_storage_t)ptc_bdb_free_storage,
			NULL TSRMLS_CC);
	retval.handlers = &ptc_bdb_handlers;

	return retval;
}
/* }}} */

/* {{{ ptc_bdbcur_new() */
static zend_object_value
ptc_bdbcur_new(zend_class_entry *ce TSRMLS_DC)
{
	zend_object_value retval;
	ptc_bdbcur *obj;

	obj = (ptc_bdbcur *)ecalloc(1, sizeof(ptc_bdbcur));
	obj->next = tcbdbcurnext;
	obj->rewind = tcbdbcurfirst;

	zend_object_std_init(&obj->std, ce TSRMLS_CC);
	object_properties_init(&obj->std, ce);

	retval.handle = zend_objects_store_put(obj,
			(zend_objects_store_dtor_t)zend_objects_destroy_object,
			(zend_objects_free_object_storage_t)ptc_bdbcur_free_storage,
			NULL TSRMLS_CC);
	retval.handlers = &ptc_bdbcur_handlers;

	return retval;
}
/* }}} */

/* {{{ ptc_bdb_free_storage() */
static void
ptc_bdb_free_storage(void *object TSRMLS_DC)
{
	ptc_db *obj = (ptc_db *)object;
	ptc_compar *cmp = (ptc_compar *)obj->op;

	if (obj->db) {
		tcbdbdel((TCBDB *)obj->db);
	}

	if (cmp->func) {
		zval_ptr_dtor(&cmp->func);
	}
	if (cmp->ctx) {
		zval_ptr_dtor(&cmp->ctx);
	}
	efree(cmp);

	zend_object_std_dtor(&obj->std TSRMLS_CC);

	efree(object);
}
/* }}} */

/* {{{ ptc_bdbcur_free_storage() */
static void
ptc_bdbcur_free_storage(void *object TSRMLS_DC)
{
	ptc_bdbcur *obj = (ptc_bdbcur *)object;

	if (obj->cur) {
		tcbdbcurdel(obj->cur);
	}

	zval_ptr_dtor(&obj->z);

	zend_object_std_dtor(&obj->std TSRMLS_CC);

	efree(object);
}
/* }}} */

/* {{{ ptc_cmp_wrapper() */
static int
ptc_cmp_wrapper(const char *aptr, int asiz, const char *bptr, int bsiz, void *op)
{
	ptc_compar *cmp = (ptc_compar *)op;
	zval *rzv, *azv, *bzv, **params[3];
	zend_fcall_info fci;
	int rv = 0;
	PTC_OP_TSRMLS_FETCH(cmp);

	if (EG(exception) != NULL) {
		return 0;
	}

	MAKE_STD_ZVAL(azv);
	MAKE_STD_ZVAL(bzv);
	ZVAL_STRINGL(azv, (char *)aptr, asiz, 1);
	ZVAL_STRINGL(bzv, (char *)bptr, bsiz, 1);

	fci = cmp->fci;
	rzv = NULL;
	params[0] = &azv;
	params[1] = &bzv;
	if (cmp->ctx != NULL) {
		params[2] = &cmp->ctx;
		fci.param_count = 3;
		fci.no_separation = 0;
	} else {
		fci.param_count = 2;
		fci.no_separation = 1;
	}
	fci.params = params;
	fci.retval_ptr_ptr = &rzv;

	//php_set_error_handling(EH_SUPPRESS, NULL TSRMLS_CC);
	if (FAILURE == zend_call_function(&fci, &(cmp->fcc) TSRMLS_CC)) {
		//php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);
		zval_ptr_dtor(&azv);
		zval_ptr_dtor(&bzv);
		return 0;
	}
	//php_set_error_handling(EH_NORMAL, NULL TSRMLS_CC);

	if (rzv != NULL) {
		if (Z_TYPE_P(rzv) != IS_LONG) {
			convert_to_long(rzv);
		}
		rv = (int)Z_LVAL_P(rzv);
		zval_ptr_dtor(&rzv);
	}

	zval_ptr_dtor(&azv);
	zval_ptr_dtor(&bzv);

	return rv;
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
