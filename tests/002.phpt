--TEST--
env: Load simple INI file
--SKIPIF--
<?php if (!extension_loaded("env")) print "skip"; ?>
--INI--
env.file={PWD}/002.ini
variables_order=EGPCS
--FILE--
<?php
echo getenv("FOO") . "\n";
echo $_SERVER['FOO'] . "\n";
echo $_ENV['FOO'] . "\n";
--EXPECT--
BAR
BAR
BAR
