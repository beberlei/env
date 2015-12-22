# env

PHP extension that parses INI file with environment variables on module startup
and populates `$_SERVER` and `$_ENV` with this information.

This allows you to easily propagate configuration values to *all* PHP executions,
following the [12factor app on Config](http://12factor.net/config).

## Installation

Regular PHP extension installation procedure:

    $ phpize
    $ ./configure
    $ make
    $ sudo make install

## Configuration

You configure a global configuration file that is used for all invocations of
PHP through the CLI or any other SAPI (Apache, FPM) for every request.

    env.file=/etc/php5/.env

## Credits

Inspiration and parts of the code are taken from laruences
[yaconf](https://pecl.php.net/package/yaconf) extension.
