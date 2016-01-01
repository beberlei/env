--TEST--
env: Load complex INI file errors
--SKIPIF--
<?php if (!extension_loaded("env") || PHP_VERSION_ID < 70000) print "skip"; ?>
--INI--
env.file={PWD}/003.ini
--FILE--
<?php
--EXPECTF--
Warning: env: parsing '%s/tests/003.ini' failed in Unknown on line 0
