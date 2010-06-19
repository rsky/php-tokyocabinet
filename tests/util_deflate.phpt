--TEST--
TCUtil::deflate() and TCUtil::inflate()
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
$data = file_get_contents(dirname(__FILE__) . '/spex-en.html');
$enc = TCUtil::deflate($data);
$dec = TCUtil::inflate($enc);
echo (strlen($enc) < strlen($data) && ($dec == $data) ? 'OK' : 'NG'), "\n";
?>
--EXPECT--
OK
