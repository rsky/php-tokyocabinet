--TEST--
TCUtil::packencode() and TCUtil::packdecode()
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
$data = 'aaaaaaaaaa';
$enc = TCUtil::packencode($data);
$dec = TCUtil::packdecode($enc);
echo (strlen($enc) < strlen($data) && ($dec == $data) ? 'OK' : 'NG'), "\n";
?>
--EXPECT--
OK
