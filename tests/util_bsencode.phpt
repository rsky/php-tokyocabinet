--TEST--
TCUtil::bsencode() and TCUtil::bsdecode()
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
$data = file_get_contents(dirname(__FILE__) . '/spex-en.html');
$enc = TCUtil::bsencode($data);
$dec = TCUtil::bsdecode($enc);
echo (strlen($enc) < strlen($data) && ($dec == $data) ? 'OK' : 'NG'), "\n";
?>
--EXPECT--
OK
