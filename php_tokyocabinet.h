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

#ifndef __PHP_TOKYOCABINET_H__
#define __PHP_TOKYOCABINET_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <php.h>
#include <php_ini.h>
#include <SAPI.h>
#include <ext/standard/info.h>
#include <Zend/zend_extensions.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_interfaces.h>
#include <tcbdb.h>
#include <tchdb.h>
#include <tcfdb.h>
#include <tcutil.h>

BEGIN_EXTERN_C()

#define PTC_MODULE_VERSION "0.4.0"

#if defined(__GNUC__) && __GNUC__ >= 4
#define PTC_LOCAL __attribute__((visibility("hidden")))
#else
#define PTC_LOCAL
#endif

#if defined(HAVE_TOKYODYSTOPIA) && HAVE_TOKYODYSTOPIA
#define PTC_ENABLE_DYSTOPIA 1
#else
#define PTC_ENABLE_DYSTOPIA 0
#endif

#if defined(HAVE_TOKYOTYRANT) && HAVE_TOKYOTYRANT
#define PTC_ENABLE_TYRANT 1
#else
#define PTC_ENABLE_TYRANT 0
#endif

#if PHP_VERSION_ID < 50300
#define ARG_INFO_STATIC static
#else
#define ARG_INFO_STATIC
#endif

/* {{{ constants */

#define PTC_IS_BINARY       0
#define PTC_IS_UTF8         1
#define PTC_IS_UNICODE      2
#define PTC_IS_INT32        3
#define PTC_IS_INT64        4
#define PTC_IS_SERIALIZED   5
#define PTC_IS_HASHTABLE    6
#define PTC_IS_ZVAL         7

#define PTC_PARAM_FAILURE   -1
#define PTC_PARAM_SUCCESS   0
#define PTC_PARAM_FREE_KBUF 1
#define PTC_PARAM_FREE_VBUF 2

#define PTC_PSEUDO_TTCCBS   (1 << 7)

typedef enum _ptc_otype {
	PTC_ADB         = 0x00,
	PTC_BDB         = 0x10,
	PTC_HDB         = 0x20,
	PTC_MDB         = 0x30,
	PTC_FDB         = 0x40,
	PTC_IDB         = 0x80,
	PTC_JDB         = 0x90,
	PTC_QDB         = 0xA0,
	PTC_WDB         = 0xB0,
	PTC_RDB         = 0xF0,
	PTC_BDBCUR      = 0x11,
	PTC_BDBITER     = 0x12,
	PTC_BDBITER_R   = 0x13
} ptc_otype;

typedef enum _ptc_ktype {
	PTC_KEY_IS_BINARY       = PTC_IS_BINARY,
	PTC_KEY_IS_INT32        = PTC_IS_INT32,
	PTC_KEY_IS_INT64        = PTC_IS_INT64,
	PTC_KEY_IS_ZVAL         = PTC_IS_ZVAL
} ptc_ktype;

typedef enum _ptc_vtype {
	PTC_VALUE_IS_BINARY     = PTC_IS_BINARY,
	PTC_VALUE_IS_UTF8       = PTC_IS_UTF8,
	PTC_VALUE_IS_SERIALIZED = PTC_IS_SERIALIZED,
	PTC_VALUE_IS_HASHTABLE  = PTC_IS_HASHTABLE,
	PTC_VALUE_IS_ZVAL       = PTC_IS_ZVAL
} ptc_vtype;

typedef enum _ptc_ecode {
	PTC_E_INVALID           = 0x1000,
	PTC_E_PARAM             = 0x1001,
	PTC_E_UNSUPPORTED       = 0x1002,
	PTC_E_CONVERSION        = 0x1003,
	PTC_E_MISC              = 0xFFFF
} ptc_ecode;

typedef enum _ptc_emode {
	PTC_ERRMODE_SILENT      = 0,
	PTC_ERRMODE_WARNING     = 1,
	PTC_ERRMODE_EXCEPTION   = 2
} ptc_emode;

typedef enum _ptc_bdb_builtin_cmp {
	PTC_BDB_CMP_LEXICAL     = 0,
	PTC_BDB_CMP_DECIMAL     = 1,
	PTC_BDB_CMP_INT32       = 2,
	PTC_BDB_CMP_INT64       = 3
} ptc_bdb_builtin_cmp;

typedef struct _ptc_deps {
	zend_class_entry *base;
	zend_class_entry *iterator;
	zend_class_entry *iteratoraggregate;
	zend_class_entry *arrayaccess;
	zend_class_entry *countable;
} ptc_deps;

typedef union _ptc_key {
	struct {
		void *buf;
		int siz;
	} str;
	int32_t i32;
	uint32_t u32;
	int64_t i64;
	uint64_t u64;
} ptc_key;

typedef const char *(*ptc_errmsg_func_t)(int ecode);
typedef void *(*ptc_new_func_t)(void);
typedef void (*ptc_del_func_t)(void *db);
typedef int (*ptc_ecode_func_t)(void *db);
typedef bool (*ptc_setmutex_func_t)(void *db);
typedef bool (*ptc_open_func_t)(void *db, const char *path, int omode);
typedef bool (*ptc_close_func_t)(void *db);
typedef bool (*ptc_sync_func_t)(void *db);
typedef bool (*ptc_vanish_func_t)(void *db);
typedef bool (*ptc_copy_func_t)(void *db, const char *path);
typedef const char *(*ptc_path_func_t)(void *db);
typedef uint64_t (*ptc_rnum_func_t)(void *db);
typedef uint64_t (*ptc_fsiz_func_t)(void *db);

typedef bool (*ptc_sput_func_t)(void *db, const void *kbuf, int ksiz, const void *vbuf, int vsiz);
typedef bool (*ptc_iput_func_t)(void *db, int64_t id, const void *vbuf, int vsiz);
typedef bool (*ptc_sout_func_t)(void *db, const void *kbuf, int ksiz);
typedef bool (*ptc_iout_func_t)(void *db, int64_t id);
typedef void *(*ptc_sget_func_t)(void *db, const void *kbuf, int ksiz, int *sp);
typedef void *(*ptc_iget_func_t)(void *db, int64_t id, int *sp);
typedef int (*ptc_svsiz_func_t)(void *db, const void *kbuf, int ksiz);
typedef int (*ptc_ivsiz_func_t)(void *db, int64_t id);
typedef int (*ptc_saddint_func_t)(void *db, const void *kbuf, int ksiz, int num);
typedef int (*ptc_iaddint_func_t)(void *db, int64_t id, int num);
typedef double (*ptc_sadddouble_func_t)(void *db, const void *kbuf, int ksiz, double num);
typedef double (*ptc_iadddouble_func_t)(void *db, int64_t id, double num);

typedef bool (*ptc_iterinit_func_t)(void *db);
typedef void *(*ptc_siternext_func_t)(void *db, int *sp);
typedef uint64_t (*ptc_iiternext_func_t)(void *db);
typedef TCLIST *(*ptc_fwmkeys_func_t)(void *db, const void *pbuf, int psiz, int max);

typedef int64_t (*ptc_parse_id_t)(const char *kbuf, int ksiz);

typedef struct _ptc_strkey_functions {
	ptc_sput_func_t put;
	ptc_sput_func_t putkeep;
	ptc_sput_func_t putcat;
	ptc_sout_func_t out;
	ptc_sget_func_t get;
	ptc_svsiz_func_t vsiz;
	ptc_saddint_func_t addint;
	ptc_sadddouble_func_t adddouble;
	ptc_siternext_func_t iternext;
} ptc_strkey_functions;

typedef struct _ptc_intkey_functions {
	ptc_iput_func_t put;
	ptc_iput_func_t putkeep;
	ptc_iput_func_t putcat;
	ptc_iout_func_t out;
	ptc_iget_func_t get;
	ptc_ivsiz_func_t vsiz;
	ptc_iaddint_func_t addint;
	ptc_iadddouble_func_t adddouble;
	ptc_iiternext_func_t iternext;
} ptc_intkey_functions;

typedef struct _ptc_functions {
	ptc_new_func_t dbnew;
	ptc_del_func_t del;
	ptc_errmsg_func_t errmsg;
	ptc_ecode_func_t ecode;
	ptc_setmutex_func_t setmutex;
	ptc_open_func_t open;
	ptc_close_func_t close;
	ptc_sync_func_t sync;
	ptc_vanish_func_t vanish;
	ptc_copy_func_t copy;
	ptc_path_func_t path;
	ptc_rnum_func_t rnum;
	ptc_fsiz_func_t fsiz;
	ptc_iterinit_func_t iterinit;
	ptc_fwmkeys_func_t fwmkeys;
	union {
		ptc_strkey_functions s;
		ptc_intkey_functions i;
	} u;
} ptc_functions;

typedef struct _ptc_compar {
	zval *func;
	zval *ctx;
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
#ifdef ZTS
	void ***tsrm_ls;
#endif
} ptc_compar;

typedef struct _ptc_db {
	zend_object std;
	ptc_otype type;
	void *db;
	void *op;
	zend_bool numeric_key;
	ptc_functions *functions;
	ptc_parse_id_t parse_id;
	ptc_ktype ktype;
	ptc_vtype vtype;
	ptc_emode emode;
	ptc_key mkey;
	ptc_key ikey;
} ptc_db;

typedef struct _ptc_bdbcur {
	zend_object std;
	BDBCUR *cur;
	bool (*next)(BDBCUR *);
	bool (*rewind)(BDBCUR *);
	bool valid;
	ptc_db *p;
	zval *z;
} ptc_bdbcur;

/* }}} */

/* {{{ function macros */

#ifdef ZTS
#define PTC_OP_TSRMLS_FETCH(op) void ***tsrm_ls = (op)->tsrm_ls
#define PTC_OP_TSRMLS_SET(op) (op)->tsrm_ls = tsrm_ls
#else
#define PTC_OP_TSRMLS_FETCH(op)
#define PTC_OP_TSRMLS_SET(op)
#endif

#define PTC_ECODE(obj) (obj)->functions->ecode((obj)->db)

#define get_object(pz) \
	zend_object_store_get_object((pz) TSRMLS_CC)

#define PTC_DB_DECLARE_OBJECT_EX(obj, pz) \
	ptc_db *obj = (ptc_db *)get_object(pz)

#define PTC_DB_DECLARE_OBJECT(obj) \
	PTC_DB_DECLARE_OBJECT_EX((obj), getThis())

#define parse_parameters(fmt, ...) \
	zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, fmt, __VA_ARGS__)

#define raise_error(code, ...) \
	php_error_docref(NULL TSRMLS_CC, code, __VA_ARGS__)

#define throw_exception(ce, code, fmt, ...) \
	(void)zend_throw_exception_ex(ce, code TSRMLS_CC, fmt, __VA_ARGS__)

#define ptc_class_exists(lcname) \
	zend_hash_exists(CG(class_table), lcname, sizeof(lcname))
#define ptc_class_find(lcname, ppce) \
	zend_hash_find(CG(class_table), lcname, sizeof(lcname), (void **)(ppce))

/* }}} */

/* {{{ inline functions */

#ifndef array_init_size
#define array_init_size(arg, size) \
	_array_init_size((arg), (size) ZEND_FILE_LINE_CC)
static inline int
_array_init_size(zval *arg, uint size ZEND_FILE_LINE_DC)
{
	HashTable *ht;
	ALLOC_HASHTABLE_REL(ht);
	if (zend_hash_init(ht, size, NULL, ZVAL_PTR_DTOR, 0) == FAILURE) {
		return FAILURE;
	}
	Z_TYPE_P(arg) = IS_ARRAY;
	Z_ARRVAL_P(arg) = ht;
	return SUCCESS;
}
#endif

/* }}} */

/* {{{ cross-object internal function prototypes */

PTC_LOCAL const char *
ptc_errmsg(int ecode);

PTC_LOCAL void
ptc_raise_error(ptc_emode emode, int ecode TSRMLS_DC),
ptc_raise_error_ex(ptc_emode emode, int ecode, const char *errmsg TSRMLS_DC);

PTC_LOCAL zend_class_entry
*ptc_get_ce_tcadb(void),
*ptc_get_ce_tcexception(void);

/* not implemented
PTC_LOCAL zend_object_iterator *
ptc_get_iterator(zend_class_entry *ce, zval *object, int by_ref TSRMLS_DC);
*/

PTC_LOCAL int
class_init_TCADB(ptc_deps *deps TSRMLS_DC),
class_init_TCBDB(ptc_deps *deps TSRMLS_DC),
class_init_TCFDB(ptc_deps *deps TSRMLS_DC),
class_init_TCHDB(ptc_deps *deps TSRMLS_DC),
class_init_TCIDB(ptc_deps *deps TSRMLS_DC),
class_init_TCJDB(ptc_deps *deps TSRMLS_DC),
class_init_TCMDB(ptc_deps *deps TSRMLS_DC),
class_init_TCQDB(ptc_deps *deps TSRMLS_DC),
class_init_TCRDB(ptc_deps *deps TSRMLS_DC),
class_init_TCWDB(ptc_deps *deps TSRMLS_DC),
class_init_TCUtil(TSRMLS_D),
class_init_TCException(TSRMLS_D);

/* }}} */

END_EXTERN_C()

#endif /* __PHP_TOKYOCABINET_H__ */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
