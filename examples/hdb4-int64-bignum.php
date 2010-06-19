<?php
extension_loaded('tokyocabinet') || dl('tokyocabinet.so') || exit(1);

try {
    $hdb = new TCHDB(TCBDB::KTINT64);
    $hdb->open('casket-i64f.hdb', TCHDB::OWRITER | TCHDB::OCREAT);

    // big number (more than 2147483647)
    // On 32-bit system, will be treated as a float.
    // On 64-bit system, will be treated as an integer.
    $hdb->put(98765432102, 'hop');
    $hdb->put(98765432100, 'step');
    $hdb->put(98765432101, 'jump');

    echo $hdb->get(98765432102.0), "\n";

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
