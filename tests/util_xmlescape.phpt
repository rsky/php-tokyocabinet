--TEST--
TCUtil::xmlescape() and TCUtil::xmlunescape()
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
$enc = TCUtil::xmlescape('<ENJOY & EXCITING>');
$str = TCUtil::xmlunescape($enc);
echo $enc, "\n";
echo $str, "\n";
?>
--EXPECT--
&lt;ENJOY &amp; EXCITING&gt;
<ENJOY & EXCITING>
