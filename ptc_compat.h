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

#ifndef __PTC_COMPAT_H__
#define __PTC_COMPAT_H__

#include "php_tokyocabinet.h"
#include <ext/standard/php_smart_str.h>
#include <ext/standard/php_var.h>
#include <tcfdb.h>

BEGIN_EXTERN_C()

#ifndef Z_ADDREF_P
#define Z_ADDREF_P(pz) ZVAL_ADDREF(pz)
#endif

#ifndef SIZEOF_INT32
#define SIZEOF_INT32 sizeof(int32_t)
#endif
#ifndef SIZEOF_INT64
#define SIZEOF_INT64 sizeof(int64_t)
#endif

#define atoi32(nptr) (int32_t)atoi(nptr)
#define atoi64(nptr) (int64_t)atoll(nptr)

#ifdef ZVAL_ASCII_STRINGL
#define PTC_ZVAL_NUMERIC_STRINGL_COPY(pzv, str, len) \
	ZVAL_ASCII_STRINGL((pzv), (str), (len), ZSTR_DUPLICATE)
#else
#define PTC_ZVAL_NUMERIC_STRINGL_COPY(pzv, str, len) \
	ZVAL_STRINGL((pzv), (str), (len), 1)
#endif

#if SIZEOF_LONG > 8 
#define ZVAL_INT64(pzv, num) ZVAL_LONG((pzv), (long)(num))
#define ZVAL_UINT64(pzv, num) ZVAL_LONG((pzv), (long)(num))
#elif SIZEOF_LONG == 8 
#define ZVAL_INT64(pzv, num) ZVAL_LONG((pzv), (long)(num))
#define ZVAL_UINT64(pzv, num) { \
	unsigned long long _ull = (unsigned long long)(num); \
	if (_ull & 0xF000000000000000ULL) { \
		char buf[24]; \
		int len = snprintf(buf, 24, "%llu", _ull); \
		PTC_ZVAL_NUMERIC_STRINGL_COPY((pzv), buf, len); \
	} else { \
		ZVAL_LONG((pzv), (long)(num)); \
	} \
}
#else
#define ZVAL_INT64(pzv, num) { \
	long long _ll = (long long)(num); \
	if (_ll > 0x7FFFFFFFLL || _ll < -0x80000000LL) { \
		char buf[24]; \
		int len = snprintf(buf, 24, "%lld", _ll); \
		PTC_ZVAL_NUMERIC_STRINGL_COPY((pzv), buf, len); \
	} else { \
		ZVAL_LONG((pzv), (long)(num)); \
	} \
}
#define ZVAL_UINT64(pzv, num) { \
	unsigned long long _ull = (unsigned long long)(num); \
	if (_ull > 0x7FFFFFFFULL) { \
		char buf[24]; \
		int len = snprintf(buf, 24, "%llu", _ull); \
		PTC_ZVAL_NUMERIC_STRINGL_COPY((pzv), buf, len); \
	} else { \
		ZVAL_LONG((pzv), (long)(num)); \
	} \
}
#endif

#define RETVAL_INT64(num) ZVAL_INT64(return_value, (num))
#define RETVAL_UINT64(num) ZVAL_UINT64(return_value, (num))
#define RETURN_INT64(num) { RETVAL_INT64(num); return; }
#define RETURN_UINT64(num) { RETVAL_UINT64(num); return; }

#define PTC_FREE_SERIALIZED(vtype, vbuf) \
	if ((vtype) == PTC_VALUE_IS_SERIALIZED) { efree(vbuf); }

#define ptc_parse_param_id(obj) \
	ptc_parse_param_id_ex((obj), 0, ZEND_NUM_ARGS() TSRMLS_CC)

#define ptc_parse_param_key(obj, kbuf_p, ksiz_p, ktype) \
	ptc_parse_param_key_ex((obj), 0, ZEND_NUM_ARGS(), \
	                       (kbuf_p), (ksiz_p), (ktype) TSRMLS_CC)

#define ptc_parse_param_id_value(obj, vbuf_p, vsiz_p, vtype) \
	ptc_parse_param_id_value_ex((obj), 0, ZEND_NUM_ARGS(), \
	                            (vbuf_p), (vsiz_p), (vtype) TSRMLS_CC)

#define ptc_parse_param_key_value(obj, kbuf_p, ksiz_p, ktype, vbuf_p, vsiz_p, vtype) \
	ptc_parse_param_key_value_ex((obj), 0, ZEND_NUM_ARGS(), \
	                             (kbuf_p), (ksiz_p), (ktype), \
	                             (vbuf_p), (vsiz_p), (vtype) TSRMLS_CC)

#define ptc_get_zval_key(ppzv, kbuf_p, ksiz_p, ktype) \
	ptc_get_zval_key_ex((ppzv), (kbuf_p), (ksiz_p), (ktype) TSRMLS_CC)

#define ptc_get_zval_value(ppzv, vbuf_p, vsiz_p, vtype) \
	ptc_get_zval_value_ex((ppzv), (vbuf_p), (vsiz_p), (vtype) TSRMLS_CC)

#define ptc_set_zval_key(pzv, kbuf, ksiz, ktype) \
	ptc_set_zval_key_ex((pzv), (kbuf), (ksiz), (ktype) TSRMLS_CC)

#define ptc_set_zval_value(pzv, vbuf, vsiz, vtype) \
	ptc_set_zval_value_ex((pzv), (vbuf), (vsiz), (vtype) TSRMLS_CC)

#define ptc_set_retval_key(kbuf, ksiz, ktype) \
	ptc_set_zval_key_ex(return_value, (kbuf), (ksiz), (ktype) TSRMLS_CC)

#define ptc_set_retval_value(vbuf, vsiz, vtype) \
	ptc_set_zval_value_ex(return_value, (vbuf), (vsiz), (vtype) TSRMLS_CC)

/*static inline uint32_t
ptc_swapint32(uint32_t num)
{
	return (((num & 0xFF000000UL) >> 24) |
	        ((num & 0x00FF0000UL) >>  8) |
	        ((num & 0x0000FF00UL) <<  8) |
	        ((num & 0x000000FFUL) << 24));
}*/

/*static inline uint64_t
ptc_swapint64(uint64_t num)
{
	return (((num & 0xFF00000000000000ULL) >> 56) |
	        ((num & 0x00FF000000000000ULL) >> 40) |
	        ((num & 0x0000FF0000000000ULL) >> 24) |
	        ((num & 0x000000FF00000000ULL) >>  8) |
	        ((num & 0x00000000FF000000ULL) <<  8) |
	        ((num & 0x0000000000FF0000ULL) << 24) |
	        ((num & 0x000000000000FF00ULL) << 40) |
	        ((num & 0x00000000000000FFULL) << 56));
}*/

#define ptc_compat_parse_parameters(type_spec, ...) \
	zend_parse_parameters_ex(flags, num_args TSRMLS_CC, (type_spec), __VA_ARGS__)

PTC_LOCAL int
ptc_parse_param_id_ex(ptc_db *obj, int flags, int num_args TSRMLS_DC);

PTC_LOCAL int
ptc_parse_param_key_ex(ptc_db *obj, int flags, int num_args,
                       void **kbuf, int *ksiz, ptc_ktype ktype TSRMLS_DC);

PTC_LOCAL int
ptc_parse_param_id_value_ex(ptc_db *obj, int flags, int num_args,
                            void **vbuf, int *vsiz, ptc_vtype vtype TSRMLS_DC);

PTC_LOCAL int
ptc_parse_param_key_value_ex(ptc_db *obj, int flags, int num_args,
                             void **kbuf, int *ksiz, ptc_ktype ktype,
                             void **vbuf, int *vsiz, ptc_vtype vtype TSRMLS_DC);

PTC_LOCAL int
ptc_get_zval_key_ex(zval **ppzv, void **kbuf, int *ksiz, ptc_vtype ktype TSRMLS_DC);

PTC_LOCAL int
ptc_get_zval_value_ex(zval **ppzv, void **vbuf, int *vsiz, ptc_vtype vtype TSRMLS_DC);

PTC_LOCAL int
ptc_set_zval_key_ex(zval *pzv, const void *kbuf, int ksiz, ptc_ktype ktype TSRMLS_DC);

PTC_LOCAL int
ptc_set_zval_value_ex(zval *pzv, const void *vbuf, int vsiz, ptc_vtype vtype TSRMLS_DC);

END_EXTERN_C()

#endif /* __PTC_COMPAT_H__ */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
