<?php
extension_loaded('tokyocabinet') || dl('tokyocabinet.so') || exit(1);

try {
    $bdb = new TCBDB();
    $bdb->open('casket.bdb', TCBDB::OWRITER | TCBDB::OCREAT);

    $bdb->put('foo', 'hop');
    $bdb->put('bar', 'step');
    $bdb->put('baz', 'jump');

    echo $bdb->get('foo'), "\n";

    echo "-\n";

    foreach ($bdb as $key => $value) {
        printf("%s:%s\n", $key, $value);
    }

    echo "-\n";

    $cur = new TCBDBCUR($bdb);
    $cur->jump('bar');
    vprintf("%s:%s\n", $cur->rec());

    echo "-\n";

    printf("%s (%d records, %d bytes)\n",
           $bdb->path(), $bdb->rnum(), $bdb->fsiz());
} catch (TCException $e) {
    echo $e->getMessage(), "\n";
}
