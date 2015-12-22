PHP_ARG_ENABLE(env, whether to enable env support,
    [  --enable-env           Enable env support])

if test "$PHP_ENV" != "no"; then
  PHP_NEW_EXTENSION(env, env.c, $ext_shared)
fi
