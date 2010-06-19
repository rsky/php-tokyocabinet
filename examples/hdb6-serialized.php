<?php
extension_loaded('tokyocabinet') || dl('tokyocabinet.so') || exit(1);

try {
    $hdb = new TCHDB(TCHDB::KTBINARY, TCHDB::VTSERIALIZED);
    $hdb->open('casket-s.hdb', TCHDB::OWRITER | TCHDB::OCREAT);

    $hdb->put('foo', array('hop'));
    $hdb->put('bar', array('step', 'Step'));
    $hdb->put('baz', array('jump', 'Jump', 'JUMP'));

    print_r($hdb->get('foo'));

    echo "-\n";

    foreach ($hdb as $key => $value) {
        printf("%s:%s", $key, print_r($value, true));
    }

    echo "-\n";

    printf("%s (%d records, %d bytes)\n",
           $hdb->path(), $hdb->rnum(), $hdb->fsiz());
    $hdb->sync();
    unset($hdb); // destruct

    echo "--\n";

    $raw = new TCHDB();
    $raw->open('casket-s.hdb', TCHDB::OREADER);
    foreach ($raw as $key => $value) {
        printf("%s:%s\n", $key, $value);
    }
} catch (TCException $e) {
    echo $e->getMessage(), "\n";
}
