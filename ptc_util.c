/*
 * Tokyo Cabinet PHP Bindings - The utility API of Tokyo Cabinet
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
#include "ptc_compat.h"

/* {{{ macros */

#define PTC_U_PARAM_STRING() \
	if (parse_parameters("s", &str, &str_len) == FAILURE) { \
		return; \
	}
#define PTC_U_RETURN() return;
#define PTC_U_RETURN_ARRAY() return;

#define PTC_UTIL_ACC (ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
#define PTC_UTIL_METHOD(name) static PHP_METHOD(TCUtil, name)
#define PTC_UTIL_ME(name, arg_info) PHP_ME(TCUtil, name, arg_info, PTC_UTIL_ACC)
#define PTC_UTIL_MALIAS(name, orig, arg_info) \
	ZEND_FENTRY(name, ZEND_MN(TCUtil_##orig), arg_info, PTC_UTIL_ACC)

/* }}} */

/* {{{ type definitions */

typedef char *(*tc_encode_func_t)(const char *ptr, int size);
typedef char *(*tc_decode_func_t)(const char *str, int *sp);
typedef char *(*tc_codec_func_t)(const char *ptr, int size, int *sp);

/* }}} */

/* {{{ globals */

static zend_class_entry *ce_TCUtil;

/* }}} */

/* {{{ internal function prototypes */

static void
ptc_util_codec(INTERNAL_FUNCTION_PARAMETERS, tc_codec_func_t func),
ptc_util_encode(INTERNAL_FUNCTION_PARAMETERS, tc_encode_func_t func),
ptc_util_decode(INTERNAL_FUNCTION_PARAMETERS, tc_decode_func_t func);

/* }}} */

/* {{{ TCUtil method prototypes */

PTC_UTIL_METHOD(urlencode);
PTC_UTIL_METHOD(urldecode);
PTC_UTIL_METHOD(urlbreak);
PTC_UTIL_METHOD(urlresolve);
PTC_UTIL_METHOD(baseencode);
PTC_UTIL_METHOD(basedecode);
PTC_UTIL_METHOD(quoteencode);
PTC_UTIL_METHOD(quotedecode);
PTC_UTIL_METHOD(mimeencode);
PTC_UTIL_METHOD(mimedecode);
PTC_UTIL_METHOD(mimebreak);
PTC_UTIL_METHOD(mimeparts);
PTC_UTIL_METHOD(packencode);
PTC_UTIL_METHOD(packdecode);
PTC_UTIL_METHOD(bsencode);
PTC_UTIL_METHOD(bsdecode);
PTC_UTIL_METHOD(deflate);
PTC_UTIL_METHOD(inflate);
PTC_UTIL_METHOD(gzipencode);
PTC_UTIL_METHOD(gzipdecode);
PTC_UTIL_METHOD(getcrc);
PTC_UTIL_METHOD(xmlescape);
PTC_UTIL_METHOD(xmlunescape);
PTC_UTIL_METHOD(xmlbreak);
PTC_UTIL_METHOD(xmlattrs);

/* }}} */

/* {{{ TCUtil argument informations */

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_util_str, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_util_data, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_util_urlresolve, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, target)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_util_mimeencode, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 2)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, encname)
	ZEND_ARG_INFO(0, base)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_util_mimedecode, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(1, encname)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO_EX(ptc_arg_util_mimebreak, ZEND_SEND_BY_VAL, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(1, headers)
ZEND_END_ARG_INFO()

ARG_INFO_STATIC
ZEND_BEGIN_ARG_INFO(ptc_arg_util_mimeparts, ZEND_SEND_BY_VAL)
	ZEND_ARG_INFO(0, str)
	ZEND_ARG_INFO(0, boundary)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ Methods */

/* {{{ string TCUtil::urlencode(string str)
 * Encode a serial object with URL encoding.
 */
PTC_UTIL_METHOD(urlencode)
{
	ptc_util_encode(INTERNAL_FUNCTION_PARAM_PASSTHRU, tcurlencode);
}
/* }}} TCUtil::urlencode */

/* {{{ string TCUtil::urldecode(string str)
 * Decode a string encoded with URL encoding.
 */
PTC_UTIL_METHOD(urldecode)
{
	ptc_util_decode(INTERNAL_FUNCTION_PARAM_PASSTHRU, tcurldecode);
}
/* }}} TCUtil::urldecode */

/* {{{ array TCUtil::urlbreak(string str)
 * Break up a URL into elements.
 */
PTC_UTIL_METHOD(urlbreak)
{
	const char *str;
	int str_len;
	TCMAP *result;
	const void *kbuf, *vbuf;
	int ksiz, vsiz;

	PTC_U_PARAM_STRING();

	result = tcurlbreak(str);
	tcmapiterinit(result);
	array_init_size(return_value, (uint)tcmaprnum(result));

	while ((kbuf = tcmapiternext(result, &ksiz)) != NULL) {
		vbuf = tcmapget(result, kbuf, ksiz, &vsiz);
		add_assoc_stringl(return_value, (char *)kbuf, (char *)vbuf, vsiz, 1);
	}

	tcmapdel(result);
	PTC_U_RETURN_ARRAY();
}
/* }}} TCUtil::urlbreak */

/* {{{ string TCUtil::urlresolve(string base, string target)
 * Resolve a relative URL with an absolute URL.
 */
PTC_UTIL_METHOD(urlresolve)
{
	const char *base, *target;
	int base_len, target_len, result_len;
	char *result;

	if (parse_parameters("ss", &base, &base_len, &target, &target_len) == FAILURE) {
		return;
	}

	result = tcurlresolve(base, target);
	result_len = (int)strlen(result);
	RETVAL_STRINGL(result, result_len, 1);
	free(result);
}
/* }}} TCUtil::urlresolve */

/* {{{ string TCUtil::baseencode(string str)
 * Encode a serial object with Base64 encoding.
 */
PTC_UTIL_METHOD(baseencode)
{
	ptc_util_encode(INTERNAL_FUNCTION_PARAM_PASSTHRU, tcbaseencode);
}
/* }}} TCUtil::baseencode */

/* {{{ string TCUtil::basedecode(string str)
 * Decode a string encoded with Base64 encoding.
 */
PTC_UTIL_METHOD(basedecode)
{
	ptc_util_decode(INTERNAL_FUNCTION_PARAM_PASSTHRU, tcbasedecode);
}
/* }}} TCUtil::basedecode */

/* {{{ string TCUtil::quoteencode(string str)
 * Encode a serial object with Quoted-printable encoding.
 */
PTC_UTIL_METHOD(quoteencode)
{
	ptc_util_encode(INTERNAL_FUNCTION_PARAM_PASSTHRU, tcquoteencode);
}
/* }}} TCUtil::quoteencode */

/* {{{ string TCUtil::quotedecode(string str)
 * Decode a string encoded with Quoted-printable encoding.
 */
PTC_UTIL_METHOD(quotedecode)
{
	ptc_util_decode(INTERNAL_FUNCTION_PARAM_PASSTHRU, tcquotedecode);
}
/* }}} TCUtil::quotedecode */

/* {{{ string TCUtil::mimeencode(string str, string encname[, bool base])
 * Encode a string with MIME encoding.
 */
PTC_UTIL_METHOD(mimeencode)
{
	const char *str;
	int str_len;
	const char *encname = NULL;
	int encname_len = 0;
	zend_bool base = 1;
	char *result;

	if (parse_parameters("ss|b", &str, &str_len, &encname, &encname_len, &base) == FAILURE) {
		return;
	}

	result = tcmimeencode(str, encname, base);
	RETVAL_STRING(result, 1);
	free(result);
}
/* }}} TCUtil::mimeencode */

/* {{{ string TCUtil::mimedecode(string str[, string &encname])
 * Decode a string encoded with MIME encoding.
 */
PTC_UTIL_METHOD(mimedecode)
{
	const char *str;
	int str_len;
	zval *zencname = NULL;
	char *result, encname[32];

	if (parse_parameters("s|z", &str, &str_len, &zencname) == FAILURE) {
		return;
	}

	result = tcmimedecode(str, encname);
	if (zencname != NULL) {
		zval_dtor(zencname);
		ZVAL_STRING(zencname, encname, 1);
	}
	RETVAL_STRING(result, 1);
	free(result);
}
/* }}} TCUtil::mimedecode */

/* {{{ string TCUtil::mimebreak(string str[, array &headers])
 * Split a string of MIME into headers and the body.
 */
PTC_UTIL_METHOD(mimebreak)
{
	const char *str;
	int str_len;
	zval *zheaders = NULL;
	TCMAP *headers = NULL;
	char *result;
	int result_len;

	if (parse_parameters("s|z", &str, &str_len, &zheaders) == FAILURE) {
		return;
	}

	if (zheaders != NULL) {
		zval_dtor(zheaders);
		ZVAL_NULL(zheaders);
		headers = tcmapnew();
	}

	result = tcmimebreak(str, str_len, headers, &result_len);
	RETVAL_STRINGL(result, result_len, 1);
	free(result);

	if (headers) {
		const void *kbuf, *vbuf;
		int ksiz, vsiz;

		array_init_size(zheaders, (uint)tcmaprnum(headers));
		tcmapiterinit(headers);
		while ((kbuf = tcmapiternext(headers, &ksiz)) != NULL) {
			vbuf = tcmapiterval(kbuf, &vsiz);
			add_assoc_stringl_ex(zheaders, (char *)kbuf, ksiz + 1, (char *)vbuf, vsiz, 1);
		}
		tcmapdel(headers);
	}
}
/* }}} TCUtil::mimebreak */

/* {{{ array TCUtil::mimeparts(string str, string boundary)
 * Split multipart data of MIME into its parts.
 */
PTC_UTIL_METHOD(mimeparts)
{
	const char *str, *boundary;
	int str_len, boundary_len;
	TCLIST *parts;
	char *vbuf;
	int vsiz;

	if (parse_parameters("ss", &str, &str_len, &boundary, &boundary_len) == FAILURE) {
		return;
	}

	parts = tcmimeparts(str, str_len, boundary);
	array_init_size(return_value, (uint)tclistnum(parts));

	while ((vbuf = tclistshift(parts, &vsiz)) != NULL) {
		add_next_index_stringl(return_value, (char *)vbuf, vsiz, 1);
		free(vbuf);
	}

	tclistdel(parts);
	PTC_U_RETURN_ARRAY();
}
/* }}} TCUtil::mimeparts */

/* {{{ string TCUtil::packencode(string str)
 * Compress a serial object with Packbits encoding.
 */
PTC_UTIL_METHOD(packencode)
{
	ptc_util_codec(INTERNAL_FUNCTION_PARAM_PASSTHRU, tcpackencode);
}
/* }}} TCUtil::packencode */

/* {{{ string TCUtil::packdecode(string str)
 * Decompress a serial object compressed with Packbits encoding.
 */
PTC_UTIL_METHOD(packdecode)
{
	ptc_util_codec(INTERNAL_FUNCTION_PARAM_PASSTHRU, tcpackdecode);
}
/* }}} TCUtil::packdecode */

/* {{{ string TCUtil::bsencode(string str)
 * Compress a serial object with TCBS encoding.
 */
PTC_UTIL_METHOD(bsencode)
{
	ptc_util_codec(INTERNAL_FUNCTION_PARAM_PASSTHRU, tcbsencode);
}
/* }}} TCUtil::bsencode */

/* {{{ string TCUtil::bsdecode(string str)
 * Decompress a serial object compressed with TCBS encoding.
 */
PTC_UTIL_METHOD(bsdecode)
{
	ptc_util_codec(INTERNAL_FUNCTION_PARAM_PASSTHRU, tcbsdecode);
}
/* }}} TCUtil::bsdecode */

/* {{{ string TCUtil::deflate(string str)
 * Compress a serial object with Deflate encoding.
 */
PTC_UTIL_METHOD(deflate)
{
	ptc_util_codec(INTERNAL_FUNCTION_PARAM_PASSTHRU, tcdeflate);
}
/* }}} TCUtil::deflate */

/* {{{ string TCUtil::inflate(string str)
 * Decompress a serial object compressed with Deflate encoding.
 */
PTC_UTIL_METHOD(inflate)
{
	ptc_util_codec(INTERNAL_FUNCTION_PARAM_PASSTHRU, tcinflate);
}
/* }}} TCUtil::inflate */

/* {{{ string TCUtil::gzipencode(string str)
 * Compress a serial object with GZIP encoding.
 */
PTC_UTIL_METHOD(gzipencode)
{
	ptc_util_codec(INTERNAL_FUNCTION_PARAM_PASSTHRU, tcgzipencode);
}
/* }}} TCUtil::gzipencode */

/* {{{ string TCUtil::gzipdecode(string str)
 * Decompress a serial object compressed with GZIP encoding.
 */
PTC_UTIL_METHOD(gzipdecode)
{
	ptc_util_codec(INTERNAL_FUNCTION_PARAM_PASSTHRU, tcgzipdecode);
}
/* }}} TCUtil::gzipdecode */

/* {{{ string TCUtil::getcrc(string str)
 * Get the CRC32 checksum of a serial object.
 */
PTC_UTIL_METHOD(getcrc)
{
	const char *str;
	int str_len;
	char result[9];

	if (parse_parameters("s", &str, &str_len) == FAILURE) {
		return;
	}

	(void)snprintf(result, 9, "%08x", tcgetcrc(str, str_len));
	RETURN_STRINGL(result, 8, 1);
}
/* }}} TCUtil::getcrc */

/* {{{ string TCUtil::xmlescape(string str)
 * Escape meta characters in a string with the entity references of XML.
 */
PTC_UTIL_METHOD(xmlescape)
{
	const char *str;
	int str_len;
	char *result;

	PTC_U_PARAM_STRING();

	result = tcxmlescape(str);
	RETVAL_STRING(result, 1);
	free(result);
	PTC_U_RETURN();
}
/* }}} TCUtil::xmlescape */

/* {{{ string TCUtil::xmlunescape(string str)
 * Unescape entity references in a string of XML.
 */
PTC_UTIL_METHOD(xmlunescape)
{
	const char *str;
	int str_len;
	char *result;

	PTC_U_PARAM_STRING();

	result = tcxmlunescape(str);
	RETVAL_STRING(result, 1);
	free(result);
	PTC_U_RETURN();
}
/* }}} TCUtil::xmlunescape */

/* {{{ array TCUtil::xmlbreak(string str)
 * Split an XML string into tags and text sections.
 */
PTC_UTIL_METHOD(xmlbreak)
{
	const char *str;
	int str_len;
	TCLIST *result;
	const void *vbuf;
	int i, num, vsiz;

	PTC_U_PARAM_STRING();

	result = tcxmlbreak(str);
	num = tclistnum(result);
	array_init_size(return_value, (uint)num);

	for (i = 0; i < num; i++) {
		vbuf = tclistval(result, i, &vsiz);
		add_next_index_stringl(return_value, (char *)vbuf, vsiz, 1);
	}

	tclistdel(result);
	PTC_U_RETURN_ARRAY();
}
/* }}} TCUtil::xmlbreak */

/* {{{ array TCUtil::xmlattrs(string str)
 * Get the map of attributes of an XML tag.
 */
PTC_UTIL_METHOD(xmlattrs)
{
	const char *str;
	int str_len;
	TCMAP *result;
	const void *kbuf, *vbuf;
	int ksiz, vsiz;

	PTC_U_PARAM_STRING();

	result = tcxmlattrs(str);
	tcmapiterinit(result);
	array_init_size(return_value, (uint)tcmaprnum(result));

	while ((kbuf = tcmapiternext(result, &ksiz)) != NULL) {
		vbuf = tcmapget(result, kbuf, ksiz, &vsiz);
		add_assoc_stringl_ex(return_value, (char *)kbuf, ksiz + 1, (char *)vbuf, vsiz, 1);
	}

	tcmapdel(result);
	PTC_U_RETURN_ARRAY();
}
/* }}} TCUtil::xmlattrs */

/* }}} Methods */

/* {{{ class_init_TCUtil() */
PTC_LOCAL int
class_init_TCUtil(TSRMLS_D)
{
	zend_class_entry ce;

	zend_function_entry TCUtil_methods[] = {
		PTC_UTIL_ME(urlencode,      ptc_arg_util_data)
		PTC_UTIL_ME(urldecode,      ptc_arg_util_str)
		PTC_UTIL_ME(urlbreak,       ptc_arg_util_str)
		PTC_UTIL_ME(urlresolve,     ptc_arg_util_urlresolve)
		PTC_UTIL_ME(baseencode,     ptc_arg_util_data)
		PTC_UTIL_ME(basedecode,     ptc_arg_util_str)
		PTC_UTIL_ME(quoteencode,    ptc_arg_util_data)
		PTC_UTIL_ME(quotedecode,    ptc_arg_util_str)
		PTC_UTIL_ME(mimeencode,     ptc_arg_util_mimeencode)
		PTC_UTIL_ME(mimedecode,     ptc_arg_util_mimedecode)
		PTC_UTIL_ME(mimebreak,      ptc_arg_util_mimebreak)
		PTC_UTIL_ME(mimeparts,      ptc_arg_util_mimeparts)
		PTC_UTIL_ME(packencode,     ptc_arg_util_data)
		PTC_UTIL_ME(packdecode,     ptc_arg_util_data)
		PTC_UTIL_ME(bsencode,       ptc_arg_util_data)
		PTC_UTIL_ME(bsdecode,       ptc_arg_util_data)
		PTC_UTIL_ME(deflate,        ptc_arg_util_data)
		PTC_UTIL_ME(inflate,        ptc_arg_util_data)
		PTC_UTIL_ME(gzipencode,     ptc_arg_util_data)
		PTC_UTIL_ME(gzipdecode,     ptc_arg_util_data)
		PTC_UTIL_ME(getcrc,         ptc_arg_util_data)
		PTC_UTIL_ME(xmlescape,      ptc_arg_util_str)
		PTC_UTIL_ME(xmlunescape,    ptc_arg_util_str)
		PTC_UTIL_ME(xmlbreak,       ptc_arg_util_str)
		PTC_UTIL_ME(xmlattrs,       ptc_arg_util_str)
		{ NULL, NULL, NULL }
	};

	INIT_CLASS_ENTRY(ce, "TCUtil", TCUtil_methods);
	ce_TCUtil = zend_register_internal_class(&ce TSRMLS_CC);
	if (ce_TCUtil == NULL) {
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ ptc_util_encode() */
static void
ptc_util_encode(INTERNAL_FUNCTION_PARAMETERS, tc_encode_func_t func)
{
	const char *str;
	int str_len;
	char *result;

	if (parse_parameters("s", &str, &str_len) == FAILURE) {
		return;
	}

	result = func(str, str_len);
	if (result == NULL) {
		RETURN_NULL();
	}
	RETVAL_STRING(result, 1);
	free(result);
}
/* }}} */

/* {{{ ptc_util_decode() */
static void
ptc_util_decode(INTERNAL_FUNCTION_PARAMETERS, tc_decode_func_t func)
{
	const char *str;
	int str_len;
	char *result;
	int result_len;

	if (parse_parameters("s", &str, &str_len) == FAILURE) {
		return;
	}

	result = func(str, &result_len);
	if (result == NULL) {
		RETURN_NULL();
	}
	RETVAL_STRINGL(result, result_len, 1);
	free(result);
}
/* }}} */

/* {{{ ptc_util_codec() */
static void
ptc_util_codec(INTERNAL_FUNCTION_PARAMETERS, tc_codec_func_t func)
{
	const char *str;
	int str_len;
	char *result;
	int result_len;

	if (parse_parameters("s", &str, &str_len) == FAILURE) {
		return;
	}

	result = func(str, str_len, &result_len);
	if (result == NULL) {
		RETURN_NULL();
	}
	RETVAL_STRINGL(result, result_len, 1);
	free(result);
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
