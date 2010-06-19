<?php
$host = @getenv('TCRDB_TEST_HOST');
$port = @getenv('TCRDB_TEST_PORT');
if (!$host) {
    $host = 'localhost';
}
if (!$port) {
    $port = 1978;
}
?>
