--TEST--
TCUtil::urlencode() and TCUtil::urldecode()
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
$enc = TCUtil::urlencode('<ENJOY & EXCITING>');
$str = TCUtil::urldecode($enc);
echo $enc, "\n";
echo $str, "\n";
?>
--EXPECT--
%3CENJOY%20%26%20EXCITING%3E
<ENJOY & EXCITING>
