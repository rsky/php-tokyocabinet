--TEST--
TCUtil::quoteencode() and TCUtil::quotedecode()
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
$data = "\x0\x1\x2\x3\x4\x5\x6\x7\x8";
$enc = TCUtil::quoteencode($data);
$dec = TCUtil::quotedecode($enc);
echo $enc, "\n";
echo (($dec == $data) ? 'OK' : bin2hex($dec)), "\n";
?>
--EXPECT--
=00=01=02=03=04=05=06=07=08
OK
