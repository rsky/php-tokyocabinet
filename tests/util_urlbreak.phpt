--TEST--
TCUtil::urlbreak()
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
$result = TCUtil::urlbreak('http://user:pass@example.com:80/index.html?foo=bar');
ksort($result);
print_r($result);
?>
--EXPECT--
Array
(
    [authority] => user:pass
    [file] => index.html
    [host] => example.com
    [path] => /index.html
    [port] => 80
    [query] => foo=bar
    [scheme] => http
    [self] => http://user:pass@example.com:80/index.html?foo=bar
)
