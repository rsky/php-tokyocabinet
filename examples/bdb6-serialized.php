<?php
extension_loaded('tokyocabinet') || dl('tokyocabinet.so') || exit(1);

try {
    $bdb = new TCBDB(TCBDB::KTBINARY, TCBDB::VTSERIALIZED);
    $bdb->open('casket-s.bdb', TCBDB::OWRITER | TCBDB::OCREAT);

    $bdb->put('foo', array('hop', 'Hop', 'HOP'));
    $bdb->put('bar', array('step', 'Step'));
    $bdb->put('baz', array('jump'));

    var_dump($bdb->get('foo'));

    echo "-\n";

    foreach ($bdb as $key => $value) {
        printf("%s:%s", $key, print_r($value, true));
    }

    echo "-\n";

    printf("%s (%d records, %d bytes)\n",
           $bdb->path(), $bdb->rnum(), $bdb->fsiz());
    $bdb->sync();
    unset($bdb); // destruct

    echo "--\n";

    $raw = new TCBDB();
    $raw->open('casket-s.bdb', TCBDB::OREADER);
    foreach ($raw as $key => $value) {
        printf("%s:%s\n", $key, $value);
    }
} catch (TCException $e) {
    echo $e->getMessage(), "\n";
}
