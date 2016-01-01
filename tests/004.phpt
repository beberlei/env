--TEST--
env: Load complex INI file errors (PHP 5)
--SKIPIF--
<?php if (!extension_loaded("env") || PHP_VERSION_ID >= 70000) print "skip"; ?>
--INI--
env.file={PWD}/003.ini
--FILE--
<?php
--EXPECTF--
PHP Warning:  env: parsing '%s/tests/003.ini' failed in Unknown on line 0

Warning: env: parsing '%s/tests/003.ini' failed in Unknown on line 0
