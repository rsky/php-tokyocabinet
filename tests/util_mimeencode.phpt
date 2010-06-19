--TEST--
TCUtil::mimeencode() and TCUtil::mimedecode()
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
$str = 'Tokyo Cabinet: a modern implementation of DBM';
$base = TCUtil::mimeencode($str, 'us-ascii');
$quote = TCUtil::mimeencode($str, 'us-ascii', false);
$dec1 = TCUtil::mimedecode($base, $enc1);
$dec2 = TCUtil::mimedecode($base, $enc2);
echo $base, "\n";
echo $dec1, "\n";
echo $enc1, "\n";
echo $quote, "\n";
echo $dec2, "\n";
echo $enc2, "\n";
?>
--EXPECT--
=?us-ascii?B?VG9reW8gQ2FiaW5ldDogYSBtb2Rlcm4gaW1wbGVtZW50YXRpb24gb2YgREJN?=
Tokyo Cabinet: a modern implementation of DBM
us-ascii
=?us-ascii?Q?Tokyo Cabinet: a modern implementation of DBM?=
Tokyo Cabinet: a modern implementation of DBM
us-ascii
