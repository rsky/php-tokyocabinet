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

#ifndef __PTC_COMMON_H__
#define __PTC_COMMON_H__

#include "php_tokyocabinet.h"
#include "ptc_compat.h"

PTC_LOCAL void
ptc_db_raise_error(ptc_db *obj TSRMLS_DC);

/* {{{ macros */

#define PTC_DB_MN(name) pim_db_##name
#define PTC_DB_MN_S(name) pim_db_##name##_s
#define PTC_DB_MN_I(name) pim_db_##name##_i
#define PTC_IT_MN(name) pim_it_##name
#define PTC_AA_MN_S(name) pim_aa_##name##_s
#define PTC_AA_MN_I(name) pim_aa_##name##_i

#define PTC_DB_METHOD(name) \
	PTC_LOCAL void PTC_DB_MN(name)(INTERNAL_FUNCTION_PARAMETERS)
#define PTC_DB_METHOD_S(name) \
	PTC_LOCAL void PTC_DB_MN_S(name)(INTERNAL_FUNCTION_PARAMETERS)
#define PTC_DB_METHOD_I(name) \
	PTC_LOCAL void PTC_DB_MN_I(name)(INTERNAL_FUNCTION_PARAMETERS)
#define PTC_IT_METHOD(name) \
	PTC_LOCAL void PTC_IT_MN(name)(INTERNAL_FUNCTION_PARAMETERS)
#define PTC_AA_METHOD_S(name) \
	PTC_LOCAL void PTC_AA_MN_S(name)(INTERNAL_FUNCTION_PARAMETERS)
#define PTC_AA_METHOD_I(name) \
	PTC_LOCAL void PTC_AA_MN_I(name)(INTERNAL_FUNCTION_PARAMETERS)

#define PTC_DB_ME(name, arg_info) \
	ZEND_FENTRY(name, PTC_DB_MN(name), arg_info, ZEND_ACC_PUBLIC)
#define PTC_DB_ME_S(name, arg_info) \
	ZEND_FENTRY(name, PTC_DB_MN_S(name), arg_info, ZEND_ACC_PUBLIC)
#define PTC_DB_ME_I(name, arg_info) \
	ZEND_FENTRY(name, PTC_DB_MN_I(name), arg_info, ZEND_ACC_PUBLIC)
#define PTC_IT_ME(name) \
	ZEND_FENTRY(name, PTC_IT_MN(name), NULL, ZEND_ACC_PUBLIC)
#define PTC_AA_ME_S(name, arg_info) \
	ZEND_FENTRY(name, PTC_AA_MN_S(name), arg_info, ZEND_ACC_PUBLIC)
#define PTC_AA_ME_I(name, arg_info) \
	ZEND_FENTRY(name, PTC_AA_MN_I(name), arg_info, ZEND_ACC_PUBLIC)
#define PTC_CONTABLE_COUNT_ME() \
	ZEND_FENTRY(count, PTC_DB_MN(rnum), NULL, ZEND_ACC_PUBLIC)

#define PTC_DB_MALIAS(name, orig, arg_info) \
	ZEND_FENTRY(name, PTC_DB_MN(orig), arg_info, ZEND_ACC_PUBLIC)
#define PTC_DB_MALIAS_S(name, orig, arg_info) \
	ZEND_FENTRY(name, PTC_DB_MN_S(orig), arg_info, ZEND_ACC_PUBLIC)
#define PTC_DB_MALIAS_I(name, orig, arg_info) \
	ZEND_FENTRY(name, PTC_DB_MN_I(orig), arg_info, ZEND_ACC_PUBLIC)

/* }}} */
/* {{{ method prototypes */

PTC_DB_METHOD(ecode);
PTC_DB_METHOD(errmsg);
PTC_DB_METHOD(seterrmode);
PTC_DB_METHOD(setmutex);

PTC_DB_METHOD(open);
PTC_DB_METHOD(close);
PTC_DB_METHOD(sync);
PTC_DB_METHOD(vanish);
PTC_DB_METHOD(copy);
PTC_DB_METHOD(path);
PTC_DB_METHOD(rnum);
PTC_DB_METHOD(fsiz);

PTC_LOCAL void
PTC_DB_MN_S(put_impl)(INTERNAL_FUNCTION_PARAMETERS, ptc_sput_func_t put_func, int ignore);

PTC_LOCAL void
PTC_DB_MN_I(put_impl)(INTERNAL_FUNCTION_PARAMETERS, ptc_iput_func_t put_func, int ignore);

PTC_DB_METHOD_S(put);
PTC_DB_METHOD_I(put);
PTC_DB_METHOD_S(putkeep);
PTC_DB_METHOD_I(putkeep);
PTC_DB_METHOD_S(putcat);
PTC_DB_METHOD_I(putcat);
PTC_DB_METHOD_S(out);
PTC_DB_METHOD_I(out);
PTC_DB_METHOD_S(get);
PTC_DB_METHOD_I(get);
PTC_DB_METHOD_S(vsiz);
PTC_DB_METHOD_I(vsiz);

PTC_DB_METHOD(fwmkeys);

PTC_DB_METHOD_S(addint);
PTC_DB_METHOD_I(addint);
PTC_DB_METHOD_S(adddouble);
PTC_DB_METHOD_I(adddouble);
PTC_DB_METHOD_S(getint);
PTC_DB_METHOD_I(getint);
PTC_DB_METHOD_S(getdouble);
PTC_DB_METHOD_I(getdouble);

PTC_IT_METHOD(current);
PTC_IT_METHOD(key);
PTC_IT_METHOD(next);
PTC_IT_METHOD(rewind);
PTC_IT_METHOD(valid);

PTC_AA_METHOD_S(offsetExists);
PTC_AA_METHOD_I(offsetExists);

/* }}} */

END_EXTERN_C()

#endif /* __PTC_COMMON_H__ */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
