--TEST--
Check for env presence
--SKIPIF--
<?php if (!extension_loaded("env")) print "skip"; ?>
--FILE--
<?php 
echo "env extension is available";
?>
--EXPECT--
env extension is available
