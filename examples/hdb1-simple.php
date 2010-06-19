<?php
extension_loaded('tokyocabinet') || dl('tokyocabinet.so') || exit(1);

try {
    $hdb = new TCHDB();
    $hdb->open('casket.hdb', TCHDB::OWRITER | TCHDB::OCREAT);

    $hdb->put('foo', 'hop');
    $hdb->put('bar', 'step');
    $hdb->put('baz', 'jump');

    echo $hdb->get('foo'), "\n";

    echo "-\n";

    foreach ($hdb as $key => $value) {
        printf("%s:%s\n", $key, $value);
    }

    echo "-\n";

    printf("%s (%d records, %d bytes)\n",
           $hdb->path(), $hdb->rnum(), $hdb->fsiz());
} catch (TCException $e) {
    echo $e->getMessage(), "\n";
}
