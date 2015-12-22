PHP_ARG_ENABLE(env, whether to enable env support,
    [  --enable-env           Enable env support])

if test "$PHP_ENV" != "no"; then
    AC_MSG_CHECKING(PHP version)
    export OLD_CPPFLAGS="$CPPFLAGS"
    export CPPFLAGS="$CPPFLAGS $INCLUDES"
    AC_TRY_COMPILE([#include <php_version.h>], [
#if PHP_MAJOR_VERSION > 5
#error  PHP > 5
#endif
    ], [
        subdir=php5
        AC_MSG_RESULT([PHP 5.x])
    ], [
        subdir=php7
        AC_MSG_RESULT([PHP 7.x])
    ])
    export CPPFLAGS="$OLD_CPPFLAGS"
    ENV_SOURCES="$subdir/php_env.c env.c"

    PHP_SUBST([LIBS])
    PHP_SUBST([ENV_SHARED_ADD])
    PHP_NEW_EXTENSION(env, $ENV_SOURCES, $ext_shared)
fi
