<?php
extension_loaded('tokyocabinet') || dl('tokyocabinet.so') || exit(1);

try {
    $bdb = new TCBDB(TCBDB::KTINT64);
    $bdb->setcmpfunc(TCBDB::CMPINT64);
    $bdb->open('casket-i64f.bdb', TCBDB::OWRITER | TCBDB::OCREAT);

    // big number (more than 2147483647)
    // On 32-bit system, will be treated as a float.
    // On 64-bit system, will be treated as an integer.
    $bdb->put(98765432102, 'hop');
    $bdb->put(98765432100, 'step');
    $bdb->put(98765432101, 'jump');

    echo $bdb->get(98765432102.0), "\n";

    echo "-\n";

    foreach ($bdb as $key => $value) {
        printf("%s(%s):%s\n", gettype($key), $key, $value);
    }

    echo "-\n";

    printf("%s (%d records, %d bytes)\n",
           $bdb->path(), $bdb->rnum(), $bdb->fsiz());
} catch (TCException $e) {
    echo $e->getMessage(), "\n";
}
