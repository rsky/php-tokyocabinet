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

#include "ptc_compat.h"

/* {{{ ptc_parse_param_id_ex() */
PTC_LOCAL int
ptc_parse_param_id_ex(ptc_db *obj, int flags, int num_args TSRMLS_DC)
{
	zval **key;

	if (ptc_compat_parse_parameters("Z", &key) == FAILURE) {
		return FAILURE;
	}

	if (Z_TYPE_PP(key) == IS_LONG) {
		obj->mkey.i64 = (int64_t)Z_LVAL_PP(key);
	} else if (obj->parse_id) {
		convert_to_string_ex(key);
		obj->mkey.i64 = obj->parse_id(Z_STRVAL_PP(key), Z_STRLEN_PP(key));
	} else {
#if SIZEOF_LONG >= 8
		convert_to_long_ex(key);
		obj->mkey.i64 = (int64_t)Z_LVAL_PP(key);
#else
		convert_to_string_ex(key);
		obj->mkey.i64 = atoi64(Z_STRVAL_PP(key));
#endif
	}

	return SUCCESS;
}
/* }}} */

/* {{{ ptc_parse_param_key_ex() */
PTC_LOCAL int
ptc_parse_param_key_ex(ptc_db *obj, int flags, int num_args,
                       void **kbuf, int *ksiz, ptc_ktype ktype TSRMLS_DC)
{
	char *type_spec = (ktype == PTC_KEY_IS_BINARY) ? "s" : "Z";
	void *kptr;
	int klen;

	if (ptc_compat_parse_parameters(type_spec, &kptr, &klen) == FAILURE) {
		return FAILURE;
	}

	switch (ktype) {
	case PTC_KEY_IS_INT32:
		{
			zval **ppzv = (zval **)kptr;
			if (Z_TYPE_PP(ppzv) != IS_LONG) {
				convert_to_long_ex(ppzv);
			}
			obj->mkey.i32 = (int32_t)Z_LVAL_PP(ppzv);
			*kbuf = (void *)&obj->mkey.i32;
			*ksiz = SIZEOF_INT32;
		}
		break;

	case PTC_KEY_IS_INT64:
		{
			zval **ppzv = (zval **)kptr;
			if (Z_TYPE_PP(ppzv) == IS_LONG) {
				obj->mkey.i64 = (int64_t)Z_LVAL_PP(ppzv);
			} else {
#if SIZEOF_LONG >= 8
				convert_to_long_ex(ppzv);
				obj->mkey.i64 = (int64_t)Z_LVAL_PP(ppzv);
#else
				convert_to_string_ex(ppzv);
				obj->mkey.i64 = atoi64(Z_STRVAL_PP(ppzv));
#endif
			}
			*kbuf = (void *)&obj->mkey.i64;
			*ksiz = SIZEOF_INT64;
		}
		break;

	default:
		*kbuf = obj->mkey.str.buf = kptr;
		*ksiz = obj->mkey.str.siz = klen;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ ptc_parse_param_id_value_ex() */
PTC_LOCAL int
ptc_parse_param_id_value_ex(ptc_db *obj, int flags, int num_args,
                            void **vbuf, int *vsiz, ptc_vtype vtype TSRMLS_DC)
{
	char type_spec[4] = {'Z', 's', '\0', '\0'};
	zval **key;
	void *vptr;
	int vlen;
	zend_bool value_is_string = 1;

	switch (vtype) {
	case PTC_VALUE_IS_ZVAL:
	case PTC_VALUE_IS_SERIALIZED:
		type_spec[1] = 'Z';
		value_is_string = 0;
		break;

	case PTC_VALUE_IS_HASHTABLE:
		type_spec[1] = 'h';
		type_spec[2] = '/';
		value_is_string = 0;
		break;
	}

	if (value_is_string) {
		if (ptc_compat_parse_parameters(type_spec, &key, &vptr, &vlen) == FAILURE) {
			return FAILURE;
		}
	} else {
		if (ptc_compat_parse_parameters(type_spec, &key, &vptr) == FAILURE) {
			return FAILURE;
		}
	}

	if (Z_TYPE_PP(key) == IS_LONG) {
		obj->mkey.i64 = (int64_t)Z_LVAL_PP(key);
	} else if (obj->parse_id) {
		convert_to_string_ex(key);
		obj->mkey.i64 = obj->parse_id(Z_STRVAL_PP(key), Z_STRLEN_PP(key));
	} else {
#if SIZEOF_LONG >= 8
		convert_to_long_ex(key);
		obj->mkey.i64 = (int64_t)Z_LVAL_PP(key);
#else
		convert_to_string_ex(key);
		obj->mkey.i64 = atoi64(Z_STRVAL_PP(key));
#endif
	}

	switch (vtype) {
	case PTC_VALUE_IS_SERIALIZED:
		{
			php_serialize_data_t var_hash;
			smart_str buf = {0};
			zval **data = (zval **)vptr;

			PHP_VAR_SERIALIZE_INIT(var_hash);
			php_var_serialize(&buf, data, &var_hash TSRMLS_CC);
			PHP_VAR_SERIALIZE_DESTROY(var_hash);

			if (buf.c == NULL) {
				raise_error(E_WARNING, "unable to serialize value");
				*vbuf = NULL;
				*vsiz = -1;
				return FAILURE;
			}
			*vbuf = buf.c;
			*vsiz = buf.len;
		}
		break;

	case PTC_VALUE_IS_HASHTABLE:
		*vbuf = vptr;
		*vsiz = zend_hash_num_elements((HashTable *)vptr);
		break;

	default:
		*vbuf = vptr;
		*vsiz = vlen;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ ptc_parse_param_key_value_ex() */
PTC_LOCAL int
ptc_parse_param_key_value_ex(ptc_db *obj, int flags, int num_args,
                             void **kbuf, int *ksiz, ptc_ktype ktype,
                             void **vbuf, int *vsiz, ptc_vtype vtype TSRMLS_DC)
{
	char type_spec[4] = {'s', 's', '\0', '\0'};
	void *kptr, *vptr;
	int klen, vlen;
	zend_bool value_is_string = 1;

	if (ktype == PTC_KEY_IS_ZVAL) {
		type_spec[0] = 'Z';
	}

	switch (vtype) {
	case PTC_VALUE_IS_ZVAL:
	case PTC_VALUE_IS_SERIALIZED:
		type_spec[1] = 'Z';
		value_is_string = 0;
		break;

	case PTC_VALUE_IS_HASHTABLE:
		type_spec[1] = 'h';
		type_spec[2] = '/';
		value_is_string = 0;
		break;
	}

	if (value_is_string) {
		if (ptc_compat_parse_parameters(type_spec, &kptr, &klen, &vptr, &vlen) == FAILURE) {
			return FAILURE;
		}
	} else {
		if (ptc_compat_parse_parameters(type_spec, &kptr, &klen, &vptr) == FAILURE) {
			return FAILURE;
		}
	}

	switch (ktype) {
	case PTC_KEY_IS_INT32:
		{
			zval **ppzv = (zval **)kptr;
			if (Z_TYPE_PP(ppzv) != IS_LONG) {
				convert_to_long_ex(ppzv);
			}
			obj->mkey.i32 = (int32_t)Z_LVAL_PP(ppzv);
			*kbuf = (void *)&obj->mkey.i32;
			*ksiz = SIZEOF_INT32;
		}
		break;

	case PTC_KEY_IS_INT64:
		{
			zval **ppzv = (zval **)kptr;
			if (Z_TYPE_PP(ppzv) == IS_LONG) {
				obj->mkey.i64 = (int64_t)Z_LVAL_PP(ppzv);
			} else {
#if SIZEOF_LONG >= 8
				convert_to_long_ex(ppzv);
				obj->mkey.i64 = (int64_t)Z_LVAL_PP(ppzv);
#else
				convert_to_string_ex(ppzv);
				obj->mkey.i64 = atoi64(Z_STRVAL_PP(ppzv));
#endif
			}
			*kbuf = (void *)&obj->mkey.i64;
			*ksiz = SIZEOF_INT64;
		}
		break;

	default:
		*kbuf = obj->mkey.str.buf = kptr;
		*ksiz = obj->mkey.str.siz = klen;
	}

	switch (vtype) {
	case PTC_VALUE_IS_SERIALIZED:
		{
			php_serialize_data_t var_hash;
			smart_str buf = {0};
			zval **data = (zval **)vptr;

			PHP_VAR_SERIALIZE_INIT(var_hash);
			php_var_serialize(&buf, data, &var_hash TSRMLS_CC);
			PHP_VAR_SERIALIZE_DESTROY(var_hash);

			if (buf.c == NULL) {
				raise_error(E_WARNING, "unable to serialize value");
				*vbuf = NULL;
				*vsiz = -1;
				return FAILURE;
			}
			*vbuf = buf.c;
			*vsiz = buf.len;
		}
		break;

	case PTC_VALUE_IS_HASHTABLE:
		*vbuf = vptr;
		*vsiz = zend_hash_num_elements((HashTable *)vptr);
		break;

	default:
		*vbuf = vptr;
		*vsiz = vlen;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ ptc_get_zval_key_ex */
PTC_LOCAL int
ptc_get_zval_key_ex(zval **ppzv, void **kbuf, int *ksiz, ptc_vtype ktype TSRMLS_DC)
{
	int ztype = Z_TYPE_PP(ppzv);

	if (ktype == PTC_KEY_IS_INT32 || ktype == PTC_KEY_IS_INT64) {
		zval z;
		int use_copy = 0;

		if (ztype == IS_LONG || ztype == IS_STRING) {
			z = **ppzv;
		} else {
			zend_make_printable_zval(*ppzv, &z, &use_copy);
		}

		if (ktype == PTC_KEY_IS_INT32) {
			int32_t ival;
			if (Z_TYPE(z) == IS_LONG) {
				ival = (int32_t)Z_LVAL(z);
			} else {
				ival = atoi32(Z_STRVAL(z));
			}
			*ksiz = SIZEOF_INT32;
			*kbuf = emalloc(SIZEOF_INT32);
			(void)memcpy(*kbuf, &ival, SIZEOF_INT32);
		} else {
			int64_t ival;
			if (Z_TYPE(z) == IS_LONG) {
				ival = (int64_t)Z_LVAL(z);
			} else {
				ival = atoi64(Z_STRVAL(z));
			}
			*ksiz = SIZEOF_INT64;
			*kbuf = emalloc(SIZEOF_INT64);
			(void)memcpy(*kbuf, &ival, SIZEOF_INT64);
		}

		if (use_copy) {
			zval_dtor(&z);
		}

		return 1;
	} else {
		if (ztype == IS_STRING) {
			*kbuf = Z_STRVAL_PP(ppzv);
			*ksiz = Z_STRLEN_PP(ppzv);

			return 0;
		} else {
			zval z = **ppzv;

			zval_copy_ctor(&z);
			convert_to_string(&z);

			*kbuf = Z_STRVAL(z);
			*ksiz = Z_STRLEN(z);

			return 1;
		}
	}
}
/* }}} */

/* {{{ ptc_get_zval_value_ex */
PTC_LOCAL int
ptc_get_zval_value_ex(zval **ppzv, void **vbuf, int *vsiz, ptc_vtype vtype TSRMLS_DC)
{
	if (vtype == PTC_VALUE_IS_SERIALIZED) {
		php_serialize_data_t var_hash;
		smart_str buf = {0};

		PHP_VAR_SERIALIZE_INIT(var_hash);
		php_var_serialize(&buf, ppzv, &var_hash TSRMLS_CC);
		PHP_VAR_SERIALIZE_DESTROY(var_hash);

		if (buf.c == NULL) {
			raise_error(E_WARNING, "unable to serialize value");
			*vbuf = NULL;
			*vsiz = -1;
			return -1;
		}
		*vbuf = buf.c;
		*vsiz = buf.len;

		return 1;
	} else if (Z_TYPE_PP(ppzv) == IS_STRING) {
		*vbuf = Z_STRVAL_PP(ppzv);
		*vsiz = Z_STRLEN_PP(ppzv);

		return 0;
	} else {
		zval z = **ppzv;

		zval_copy_ctor(&z);
		convert_to_string(&z);

		*vbuf = Z_STRVAL(z);
		*vsiz = Z_STRLEN(z);

		return 1;
	}
}
/* }}} */

/* {{{ ptc_set_zval_key_ex */
PTC_LOCAL int
ptc_set_zval_key_ex(zval *pzv, const void *kbuf, int ksiz, ptc_ktype ktype TSRMLS_DC)
{
	switch (ktype) {
	case PTC_KEY_IS_INT32:
		{
			uint8_t ibuf[SIZEOF_INT32];
			(void)memset(ibuf, 0, SIZEOF_INT32);
			(void)memcpy(ibuf, kbuf, MIN(ksiz, SIZEOF_INT32));
			int32_t ival = *(int32_t *)ibuf;
			ZVAL_LONG(pzv, (long)ival);
		}
		break;

	case PTC_KEY_IS_INT64:
		{
			uint8_t ibuf[SIZEOF_INT64];
			(void)memset(ibuf, 0, SIZEOF_INT64);
			(void)memcpy(ibuf, kbuf, MIN(ksiz, SIZEOF_INT64));
			int64_t ival = *(int64_t *)ibuf;
			ZVAL_INT64(pzv, ival);
		}

	default:
		ZVAL_STRINGL(pzv, (char *)kbuf, ksiz, 1);
	}

	return SUCCESS;
}
/* }}} */

/* {{{ ptc_set_zval_value_ex */
PTC_LOCAL int
ptc_set_zval_value_ex(zval *pzv, const void *vbuf, int vsiz, ptc_vtype vtype TSRMLS_DC)
{
	if (vtype == PTC_VALUE_IS_SERIALIZED) {
		const unsigned char *ptr = (const unsigned char *)vbuf;
		php_unserialize_data_t var_hash;
		zval *data;

		ALLOC_INIT_ZVAL(data);
		PHP_VAR_UNSERIALIZE_INIT(var_hash);

		if (!php_var_unserialize(&data, &ptr, ptr + vsiz, &var_hash TSRMLS_CC)) {
			PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
			zval_ptr_dtor(&data);
			return FAILURE;
		}

		PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
		ZVAL_ZVAL(pzv, data, 0, 1);
	} else {
		ZVAL_STRINGL(pzv, (char *)vbuf, vsiz, 1);
	}

	return SUCCESS;
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
