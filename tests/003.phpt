--TEST--
env: Load complex INI file errors
--SKIPIF--
<?php if (!extension_loaded("env")) print "skip"; ?>
--INI--
env.file={PWD}/003.ini
--FILE--
<?php
--EXPECTF--
%AWarning: env: parsing '%s/tests/003.ini' failed in Unknown on line 0
