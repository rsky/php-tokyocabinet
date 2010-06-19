--TEST--
TCUtil::baseencode() and TCUtil::basedecode()
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
$enc = TCUtil::baseencode('<ENJOY & EXCITING>');
$str = TCUtil::basedecode($enc);
echo $enc, "\n";
echo $str, "\n";
?>
--EXPECT--
PEVOSk9ZICYgRVhDSVRJTkc+
<ENJOY & EXCITING>
