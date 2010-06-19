--TEST--
TCFDB (Tokyo Cabinet) all method test
--SKIPIF--
<?php
include 'skipif.inc';
if (!class_exists('TCFDB', false)) {
    die('skip class TCFDB is not available');
}
?>
--FILE--
<?php
$dbname = dirname(__FILE__) . '/casket.fdb';
$cpname = dirname(__FILE__) . '/casket2.fdb';
file_exists($dbname) && unlink($dbname);
file_exists($cpname) && unlink($cpname);

echo "-open-\n";

$fdb = new TCFDB();
var_dump($fdb->seterrmode(TCFDB::EMWARNING));
var_dump($fdb->setmutex());
var_dump($fdb->open($dbname, TCFDB::OWRITER | TCFDB::OCREAT));


echo "-put-\n";

var_dump($fdb->put(1, 'hop'));
var_dump($fdb->put(2, 'step'));
var_dump($fdb->put(3, 'jump'));
var_dump($fdb->putkeep(2, 'step'));
var_dump($fdb->putcat(3, 'jump'));


echo "-get-\n";

echo $fdb->get(1), "\n";
echo $fdb->get(5), "\n";


echo "-iterator-\n";

foreach ($fdb as $key => $value) {
    printf("%d:%s\n", $key, $value);
}


echo "-info-\n";

printf("%d\n", $fdb->vsiz(3));

printf("%s (%d/%d)\n", $fdb->path(), $fdb->rnum(), $fdb->fsiz());


echo "-out-\n";

var_dump($fdb->out(1));
var_dump($fdb->out(5));


echo "-int-double-\n";

var_dump($fdb->addint(32, 1980));
var_dump($fdb->adddouble(64, 20.35));
var_dump($fdb->getint(32));
var_dump($fdb->getdouble(64));


echo "-misc-\n";

var_dump($fdb->sync());
var_dump($fdb->optimize(20, 100)); // FIXME
printf("%d/%d\n", $fdb->rnum(), $fdb->fsiz());
if (method_exists($fdb, 'copy')) {
    var_dump($fdb->copy($cpname));
} else {
    var_dump(true);
}
var_dump($fdb->vanish());


echo "-arrayaccess-\n";

printf("%d\n", count($fdb));
var_dump(isset($fdb[6]));
$fdb[6] = 'Hypertext Preprocessor';
var_dump(isset($fdb[6]));
printf("%d\n", count($fdb));
echo $fdb[6], "\n";
unset($fdb[6]);
printf("%d\n", count($fdb));


echo "-cleanup-\n";

$fdb->sync();
var_dump($fdb->close());
unset($fdb);
file_exists($dbname) && unlink($dbname);
file_exists($cpname) && unlink($cpname);
echo "OK\n";
?>
--EXPECTREGEX--
-open-
bool\(true\)
bool\(true\)
bool\(true\)
-put-
bool\(true\)
bool\(true\)
bool\(true\)
bool\(false\)
bool\(true\)
-get-
hop

-iterator-
1:hop
2:step
3:jumpjump
-info-
8
.*?\x2Fcasket\.fdb \(3\x2F[1-9][0-9]+\)
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
4\x2F[1-9][0-9]+
bool\(true\)
bool\(true\)
-arrayaccess-
0
bool\(false\)
bool\(true\)
1
Hypertext Preprocess
0
-cleanup-
bool\(true\)
OK
