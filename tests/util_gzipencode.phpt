--TEST--
TCUtil::gzipencode() and TCUtil::gzipdecode()
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
$data = file_get_contents(dirname(__FILE__) . '/spex-en.html');
$enc = TCUtil::gzipencode($data);
$dec = TCUtil::gzipdecode($enc);
echo (strlen($enc) < strlen($data) && ($dec == $data) ? 'OK' : 'NG'), "\n";
?>
--EXPECT--
OK
