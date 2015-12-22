--TEST--
Check for env presence
--SKIPIF--
<?php if (!extension_loaded("env")) print "skip"; ?>
--INI--
env.file=/etc/php/.env
--FILE--
<?php 
echo "env extension is available\n";
echo ini_get("env.file") . "\n";
?>
--EXPECT--
env extension is available
/etc/php/.env
