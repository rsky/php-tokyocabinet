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

/* {{{ globals */

static zend_class_entry *ce_TCException;

/* }}} */

/* {{{ ptc_get_ce_tcexception() */
PTC_LOCAL zend_class_entry *
ptc_get_ce_tcexception(void)
{
	return ce_TCException;
}
/* }}} */

/* {{{ class_init_TCException() */
PTC_LOCAL int
class_init_TCException(TSRMLS_D)
{
	zend_class_entry ce, **tmp;

	if (FAILURE == zend_hash_find(CG(class_table), "runtimeexception", 17, (void **)&tmp)) {
		return FAILURE;
	}

	INIT_CLASS_ENTRY(ce, "TCException", NULL);
	ce_TCException = zend_register_internal_class_ex(&ce, *tmp, NULL TSRMLS_CC);
	if (ce_TCException == NULL) {
		return FAILURE;
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
