<?php
extension_loaded('tokyocabinet') || dl('tokyocabinet.so') || exit(1);

try {
    $hdb = new TCHDB(TCBDB::KTINT64);
    $hdb->open('casket-i64.hdb', TCHDB::OWRITER | TCHDB::OCREAT);

    $hdb->put(2, 'hop');
    $hdb->put(0, 'step');
    $hdb->put(1, 'jump');

    echo $hdb->get(2), "\n";

    echo "-\n";

    foreach ($hdb as $key => $value) {
        printf("%s(%s):%s\n", gettype($key), $key, $value);
    }

    echo "-\n";

    printf("%s (%d records, %d bytes)\n",
           $hdb->path(), $hdb->rnum(), $hdb->fsiz());
} catch (TCException $e) {
    echo $e->getMessage(), "\n";
}
