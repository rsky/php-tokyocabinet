dnl
dnl $ Id: $
dnl

PHP_ARG_ENABLE(tokyocabinet,            [whether to enable Tokyo Cabinet support],
[  --enable-tokyocabinet           Enable Tokyo Cabinet support], yes, yes)
dnl PHP_ARG_ENABLE(tokyocabinet-dystopia,   [whether to enable Tokyo Dystopia support],
dnl [  --enable-tokyocabinet-dystopia  Enable Tokyo Dystopia support], no, no)
dnl PHP_ARG_ENABLE(tokyocabinet-tyrant,     [whether to enable Tokyo Tyrant support],
dnl [  --enable-tokyocabinet-tyrant    Enable Tokyo Tyrant support], no, no)

if test "$PHP_TOKYOCABINET" = "yes"; then

  if test -z "$AWK"; then
    AC_PATH_PROGS(AWK, awk gawk nawk, [no])
  fi
  if test -z "$SED"; then
    AC_PATH_PROGS(SED, sed gsed, [no])
  fi
  if test -z "$PKG_CONFIG"; then
    AC_PATH_PROGS(PKG_CONFIG, pkg-config, [no])
  fi

  dnl
  dnl Check PHP version
  dnl
  export OLD_CPPFLAGS="$CPPFLAGS"
  export CPPFLAGS="$CPPFLAGS $INCLUDES"
  AC_MSG_CHECKING([PHP version])
  AC_TRY_COMPILE([#include <php_version.h>], [
#if !defined(PHP_VERSION_ID) || PHP_VERSION_ID < 50200
#error this extension requires at least PHP version 5.2.0
#endif
],
    [AC_MSG_RESULT([ok])],
    [AC_MSG_ERROR([need at least PHP 5.2.0])])
  export CPPFLAGS="$OLD_CPPFLAGS"

  dnl
  dnl Check Tokyo Cabinet availability
  dnl
  AC_MSG_CHECKING([for Tokyo Cabinet availability])
  if test "$PKG_CONFIG" = "no"; then
    AC_MSG_ERROR([required utility 'pkg-config' not found])
  fi

  if ! $PKG_CONFIG --exists tokyocabinet; then
    AC_MSG_ERROR(['tokyocabinet' not known to pkg-config])
  fi
  TC_PACKAGES="tokyocabinet"

  dnl if test "$PHP_TOKYOCABINET_DYSTOPIA" != "no"; then
  dnl   if ! $PKG_CONFIG --exists tokyodystopia; then
  dnl     AC_MSG_ERROR(['tokyodystopia' not known to pkg-config])
  dnl   fi
  dnl   TC_PACKAGES="$TC_PACKAGES tokyodystopia"
  dnl fi

  dnl if test "$PHP_TOKYOCABINET_TYRANT" != "no"; then
  dnl   if ! $PKG_CONFIG --exists tokyotyrant; then
  dnl     AC_MSG_ERROR(['tokyotyrant' not known to pkg-config])
  dnl   fi
  dnl   TC_PACKAGES="$TC_PACKAGES tokyotyrant"
  dnl fi

  AC_MSG_RESULT([ok])

  dnl
  dnl Get the version number, CFLAGS and LIBS by pkg-config
  dnl
  AC_MSG_CHECKING([for Tokyo Cabinet library version])
  TC_VERSION=`$PKG_CONFIG --modversion tokyocabinet`
  TC_VERSION_NUMBER=`echo $TC_VERSION | $AWK -F. '{ printf "%d", ($1 * 1000 + $2) * 1000 + $3 }'`

  if test "$TC_VERSION_NUMBER" -lt 1003006; then
    AC_MSG_RESULT([$TC_VERSION])
    AC_MSG_ERROR([Tokyo Cabinet version 1.3.6 or later is required to compile php with Tokyo Cabinet support])
  fi
  AC_MSG_RESULT([$TC_VERSION (ok)])

  dnl if test "$PHP_TOKYOCABINET_DYSTOPIA" != "no"; then
  dnl   AC_MSG_CHECKING([for Tokyo Dystopia library version])
  dnl   TD_VERSION=`$PKG_CONFIG --modversion tokyodystopia`
  dnl   TD_VERSION_NUMBER=`echo $TD_VERSION | $AWK -F. '{ printf "%d", ($1 * 1000 + $2) * 1000 + $3 }'`
  dnl   
  dnl   if test "$TD_VERSION_NUMBER" -lt 9005; then
  dnl     AC_MSG_RESULT([$TD_VERSION])
  dnl     AC_MSG_ERROR([Tokyo Dystopia version 0.9.5 or later is required to compile php with Tokyo Dystopia support])
  dnl   fi
  dnl   AC_MSG_RESULT([$TD_VERSION (ok)])
  dnl fi

  dnl if test "$PHP_TOKYOCABINET_TYRANT" != "no"; then
  dnl   AC_MSG_CHECKING([for Tokyo Tyrant library version])
  dnl   TT_VERSION=`$PKG_CONFIG --modversion tokyotyrant`
  dnl   TT_VERSION_NUMBER=`echo $TT_VERSION | $AWK -F. '{ printf "%d", ($1 * 1000 + $2) * 1000 + $3 }'`
  dnl   
  dnl   if test "$TT_VERSION_NUMBER" -lt 1000001; then
  dnl     AC_MSG_RESULT([$TT_VERSION])
  dnl     AC_MSG_ERROR([Tokyo Tyrant version 1.0.1 or later is required to compile php with Tokyo Tyrant support])
  dnl   fi
  dnl   AC_MSG_RESULT([$TT_VERSION (ok)])
  dnl fi

  PHP_EVAL_INCLINE(`$PKG_CONFIG --cflags-only-I $TC_PACKAGES`)
  PHP_EVAL_LIBLINE(`$PKG_CONFIG --libs $TC_PACKAGES`, TOKYOCABINET_SHARED_LIBADD)

  dnl
  dnl Check for headers
  dnl
  export OLD_CPPFLAGS="$CPPFLAGS"
  export CPPFLAGS="$CPPFLAGS $INCLUDES"
  AC_CHECK_HEADER([tcbdb.h], [], AC_MSG_ERROR('tcbdb.h' header not found))
  AC_CHECK_HEADER([tchdb.h], [], AC_MSG_ERROR('tchdb.h' header not found))
  AC_CHECK_HEADER([tcfdb.h], [], AC_MSG_ERROR('tcfdb.h' header not found))
  AC_CHECK_HEADER([tcutil.h], [], AC_MSG_ERROR('tcutil.h' header not found))
  dnl if test "$PHP_TOKYOCABINET_DYSTOPIA" != "no"; then
  dnl   AC_CHECK_HEADER([dystopia.h], [], AC_MSG_ERROR('dystopia.h' header not found))
  dnl   AC_CHECK_HEADER([laputa.h], [], AC_MSG_ERROR('laputa.h' header not found))
  dnl fi
  dnl if test "$PHP_TOKYOCABINET_TYRANT" != "no"; then
  dnl   AC_CHECK_HEADER([tcrdb.h], [], AC_MSG_ERROR('tcrdb.h' header not found))
  dnl fi
  export CPPFLAGS="$OLD_CPPFLAGS"

  dnl
  dnl Set the build target
  dnl
  PHP_SUBST(TOKYOCABINET_SHARED_LIBADD)
  AC_DEFINE(HAVE_TOKYOCABINET, 1, [Enable Tokyo Cabinet support])
  AC_DEFINE_UNQUOTED(TOKYOCABINET_VERSION_NUMBER, $TC_VERSION_NUMBER, [Tokyo Cabinet version number])
  TC_PHP_SOURCEFILES="tokyocabinet.c ptc_abstract.c ptc_hdb.c ptc_bdb.c ptc_fdb.c"
 
  dnl if test "$PHP_TOKYOCABINET_DYSTOPIA" != "no"; then
  dnl   AC_DEFINE(HAVE_TOKYODYSTOPIA, 1, [Enable Tokyo Dystopia support])
  dnl   AC_DEFINE_UNQUOTED(TOKYODYSTOPIA_VERSION_NUMBER, $TD_VERSION_NUMBER, [Tokyo Dystopia version number])
  dnl   TC_PHP_SOURCEFILES="$TC_PHP_SOURCEFILES ptc_idb.c ptc_jdb.c"
  dnl fi

  dnl if test "$PHP_TOKYOCABINET_TYRANT" != "no"; then
  dnl   AC_DEFINE(HAVE_TOKYOTYRANT, 1, [Enable Tokyo Tyrant support])
  dnl   AC_DEFINE_UNQUOTED(TOKYOTYRANT_VERSION_NUMBER, $TT_VERSION_NUMBER, [Tokyo Tyrant version number])
  dnl   TC_PHP_SOURCEFILES="$TC_PHP_SOURCEFILES ptc_rdb.c"
  dnl fi

  TC_PHP_SOURCEFILES="$TC_PHP_SOURCEFILES ptc_util.c ptc_exception.c"

  dnl TC_PHP_VERNUM=`$PHP_CONFIG --vernum 2>&1`
  dnl #if test "$TC_PHP_VERNUM" -lt 60000; then
    TC_PHP_SOURCEFILES="$TC_PHP_SOURCEFILES ptc_compat.c"
  dnl else
  dnl   TC_PHP_SOURCEFILES="$TC_PHP_SOURCEFILES ptc_compat6.c"
  dnl fi

  PHP_NEW_EXTENSION(tokyocabinet, $TC_PHP_SOURCEFILES, $ext_shared)

fi
