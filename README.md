# env

[![Build Status](https://travis-ci.org/beberlei/env.svg)](https://travis-ci.org/beberlei/env)

PHP extension that parses environment file on module startup and populates
`getenv()` with this information.

This allows you to easily propagate configuration values to *all* PHP executions,
following the [12factor app on Config](http://12factor.net/config).

Settings are loaded once, during PHP module startup. There is no additional
file parsing necessary during requests, because data is put into the web
servers environment variables using `setenv`.

**EXPERIMENTAL** This extension is not tested in production yet. It currently
works with PHP 5.5, 5.6 and PHP 7 on Non-ZTS builds of PHP.

**Working and tested SAPIs:**

- CLI
- Apache2
- PHP Builtin Server
- PHP-FPM

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

## Example

With an `env.file=/etc/php5/.env` of the following contents:

    FOO=BAR

You can access the information during a PHP request with:

```php
<?php

echo getenv('FOO');
```

## Integration Example: Symfony2

You can integrate this extension into the build process of a Symfony 2 Dependency Injection Container
in your `AppKernel`:

```php
<?php

use Symfony\Component\HttpKernel\Kernel;

class AppKernel extends Kernel
{
    // other methods...

    protected function getEnvParameters()
    {
        $parameters = parent::getEnvParameters();

        $parameters['DATABASE_DSN'] = $this->getenv('DATABASE_DSN');

        return $parameters;
    }

    private function getenv($name)
    {
        $value = getenv($name);

        if ($value === false) {
            throw new \RuntimeException(sprintf("Cannot build application: %s environment variable is missing.", $name));
        }

        return $value;
    }
}
```


## Credits

Inspiration and parts of the code are taken from laruences
[yaconf](https://pecl.php.net/package/yaconf) extension.

## License

MIT License
