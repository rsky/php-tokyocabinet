--TEST--
TCUtil::urlresolve()
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
echo TCUtil::urlresolve('http://tokyocabinet.sourceforge.net/', './index.ja.html');
?>
--EXPECT--
http://tokyocabinet.sourceforge.net/index.ja.html
