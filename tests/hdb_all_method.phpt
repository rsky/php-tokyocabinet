--TEST--
TCHDB (Tokyo Cabinet) all method test
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
$dbname = dirname(__FILE__) . '/casket.hdb';
$cpname = dirname(__FILE__) . '/casket2.hdb';
file_exists($dbname) && unlink($dbname);
file_exists($cpname) && unlink($cpname);

echo "-open-\n";

$hdb = new TCHDB();
var_dump($hdb->seterrmode(TCHDB::EMWARNING));
var_dump($hdb->setmutex());
var_dump($hdb->tune(1000000, 12, -1, TCHDB::TLARGE | TCHDB::TDEFLATE));
var_dump($hdb->setcache(-1));
var_dump($hdb->open($dbname, TCHDB::OWRITER | TCHDB::OCREAT));


echo "-put-\n";

var_dump($hdb->put('foo', 'hop'));
var_dump($hdb->put('bar', 'step'));
var_dump($hdb->put('baz', 'jump'));
var_dump($hdb->putkeep('bar', 'step'));
var_dump($hdb->putcat('baz', 'jump'));
var_dump($hdb->putasync('qux', 'landing'));


echo "-get-\n";

echo $hdb->get('foo'), "\n";
echo $hdb->get('quux'), "\n";


echo "-iterator-\n";

foreach ($hdb as $key => $value) {
    printf("%s:%s\n", $key, $value);
}


echo "-info-\n";

printf("%d\n", $hdb->vsiz('qux'));

printf("%s (%d/%d)\n", $hdb->path(), $hdb->rnum(), $hdb->fsiz());


echo "-keys-\n";

foreach ($hdb->fwmkeys('ba') as $key) {
    echo $key, "\n";
}


echo "-out-\n";

var_dump($hdb->out('foo'));
var_dump($hdb->out('quux'));


echo "-int-double-\n";

var_dump($hdb->addint('lval', 1980));
var_dump($hdb->adddouble('dval', 20.35));
var_dump($hdb->getint('lval'));
var_dump($hdb->getdouble('dval'));


echo "-misc-\n";

var_dump($hdb->sync());
var_dump($hdb->optimize(100, 4, -1, TCHDB::TTCBS));
printf("%d/%d\n", $hdb->rnum(), $hdb->fsiz());
if (method_exists($hdb, 'copy')) {
    var_dump($hdb->copy($cpname));
} else {
    var_dump(true);
}
var_dump($hdb->vanish());


echo "-arrayaccess-\n";

printf("%d\n", count($hdb));
var_dump(isset($hdb['php']));
$hdb['php'] = 'Hypertext Preprocessor';
var_dump(isset($hdb['php']));
printf("%d\n", count($hdb));
echo $hdb['php'], "\n";
unset($hdb['php']);
printf("%d\n", count($hdb));


echo "-cleanup-\n";

$hdb->sync();
var_dump($hdb->close());
unset($hdb);
file_exists($dbname) && unlink($dbname);
file_exists($cpname) && unlink($cpname);
echo "OK\n";
?>
--EXPECTREGEX--
-open-
bool\(true\)
bool\(true\)
bool\(true\)
bool\(true\)
bool\(true\)
-put-
bool\(true\)
bool\(true\)
bool\(true\)
bool\(false\)
bool\(true\)
bool\(true\)
-get-
hop

-iterator-
foo:hop
bar:step
baz:jumpjump
qux:landing
-info-
7
.*?\x2Fcasket\.hdb \(4\x2F[1-9][0-9]+\)
-keys-
bar
baz
-out-
bool\(true\)
bool\(false\)
-int-double-
bool\(true\)
bool\(true\)
int\(1980\)
float\(20\.35\)
-misc-
bool\(true\)
bool\(true\)
5\x2F[1-9][0-9]+
bool\(true\)
bool\(true\)
-arrayaccess-
0
bool\(false\)
bool\(true\)
1
Hypertext Preprocessor
0
-cleanup-
bool\(true\)
OK
