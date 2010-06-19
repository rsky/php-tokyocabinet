--TEST--
TCUtil::getcrc()
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
$data = file_get_contents(dirname(__FILE__) . '/spex-en.html');
echo TCUtil::getcrc($data), "\n";
?>
--EXPECT--
c366cab4
