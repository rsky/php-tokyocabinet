--TEST--
TCRDB (Tokyo Tyrant) all method test
--SKIPIF--
<?php
die('skip this test is not implemented');
include 'skipif.inc';
include 'rdbcfg.inc';
if (!($fp = @fsockopen('localhost', 1978))) {
    die('skip cannot connect to localhost:1978');
}
fclose($fp);
?>
--FILE--
<?php
include 'rdbcfg.inc';
?>
--EXPECT--
