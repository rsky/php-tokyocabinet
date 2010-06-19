--TEST--
TCBDB (Tokyo Cabinet) all method test
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
$dbname = dirname(__FILE__) . '/casket.bdb';
$cpname = dirname(__FILE__) . '/casket2.bdb';
file_exists($dbname) && unlink($dbname);
file_exists($cpname) && unlink($cpname);

echo "-open-\n";

$bdb = new TCBDB();
var_dump($bdb->seterrmode(TCBDB::EMWARNING));
var_dump($bdb->setmutex());
var_dump($bdb->setcmpfunc(TCBDB::CMPLEXICAL));
var_dump($bdb->tune(180, -1, 1000000, 12, -1, TCBDB::TLARGE | TCBDB::TDEFLATE));
var_dump($bdb->setcache(2048, 512));
var_dump($bdb->open($dbname, TCBDB::OWRITER | TCBDB::OCREAT));


echo "-put-\n";

var_dump($bdb->put('foo', 'hop'));
var_dump($bdb->put('bar', 'step'));
var_dump($bdb->put('baz', 'jump'));
var_dump($bdb->putkeep('foo', 'hop'));
var_dump($bdb->putcat('bar', 'step'));
var_dump($bdb->putdup('baz', 'landing'));
var_dump($bdb->putlist('qux', array('cogito', 'ergo', 'sum')));


echo "-get-\n";

echo $bdb->get('foo'), "\n";
echo $bdb->get('quux'), "\n";

foreach ($bdb->getlist('qux') as $baz) {
    echo $baz, "\n";
}


echo "-iterator-\n";

foreach ($bdb as $key => $value) {
    printf("%s:%s\n", $key, $value);
}

$rev = new TCBDBITER($bdb, true);
foreach ($rev as $key => $value) {
    printf("%s:%s\n", $key, $value);
}
unset($rev);


echo "-info-\n";

printf("%d/%d\n", $bdb->vnum('qux'), $bdb->vsiz('qux'));

printf("%s (%d/%d)\n", $bdb->path(), $bdb->rnum(), $bdb->fsiz());


echo "-cursor-get-\n";

$cur = new TCBDBCUR($bdb);

printf("%s:%s\n", $cur->key(), $cur->val());
var_dump($cur->last());
vprintf("%s:%s\n", $cur->rec());
var_dump($cur->prev());
vprintf("%s:%s\n", $cur->rec());
var_dump($cur->first());
vprintf("%s:%s\n", $cur->rec());
var_dump($cur->next());
vprintf("%s:%s\n", $cur->rec());
var_dump($cur->jump('foo'));
vprintf("%s:%s\n", $cur->rec());


echo "-cursor-put-\n";


var_dump($cur->put('ton', TCBDB::CPCURRENT));

$cur2 = new TCBDBCUR($bdb);
$cur2->jump('foo');
var_dump($cur2->put('tin', TCBDB::CPBEFORE));

$cur3 = new TCBDBCUR($bdb);
$cur3->jump('foo');
var_dump($cur3->put('kan', TCBDB::CPAFTER));

foreach ($bdb->getlist('foo') as $foo) {
    echo $foo, "\n";
}


echo "-cursor-out-\n";

var_dump($cur->out());

unset($cur, $cur2, $cur3);


echo "-range-\n";

foreach ($bdb->range('baz', true, 'qux', true) as $key) {
    echo $key, "\n";
}


echo "-keys-\n";

foreach ($bdb->fwmkeys('ba') as $key) {
    echo $key, "\n";
}


echo "-out-\n";

var_dump($bdb->out('bar'));
var_dump($bdb->out('quux'));
var_dump($bdb->outlist('baz'));


echo "-int-double-\n";

var_dump($bdb->addint('lval', 1980));
var_dump($bdb->adddouble('dval', 20.35));
var_dump($bdb->getint('lval'));
var_dump($bdb->getdouble('dval'));


echo "-misc-\n";

var_dump($bdb->sync());
var_dump($bdb->optimize(10, -1, 100, 4, -1, TCBDB::TTCBS));
printf("%d/%d\n", $bdb->rnum(), $bdb->fsiz());
if (method_exists($bdb, 'copy')) {
    var_dump($bdb->copy($cpname));
} else {
    var_dump(true);
}
var_dump($bdb->vanish());


echo "-arrayaccess-\n";

printf("%d\n", count($bdb));
var_dump(isset($bdb['php']));
$bdb['php'] = 'Hypertext Preprocessor';
var_dump(isset($bdb['php']));
printf("%d\n", count($bdb));
echo $bdb['php'], "\n";
unset($bdb['php']);
printf("%d\n", count($bdb));


echo "-transaction-\n";

printf("%d\n", $bdb->rnum());
var_dump($bdb->tranbegin());
var_dump($bdb->put('foo', 'hop'));
var_dump($bdb->tranabort());
printf("%d\n", $bdb->rnum());
var_dump($bdb->tranbegin());
var_dump($bdb->put('foo', 'hop'));
var_dump($bdb->trancommit());
printf("%d\n", $bdb->rnum());


echo "-cleanup-\n";

$bdb->sync();
var_dump($bdb->close());
unset($bdb);
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
bool\(true\)
-put-
bool\(true\)
bool\(true\)
bool\(true\)
bool\(false\)
bool\(true\)
bool\(true\)
bool\(true\)
-get-
hop

cogito
ergo
sum
-iterator-
bar:stepstep
baz:jump
baz:landing
foo:hop
qux:cogito
qux:ergo
qux:sum
qux:sum
qux:ergo
qux:cogito
foo:hop
baz:landing
baz:jump
bar:stepstep
-info-
3\x2F6
.*?\x2Fcasket\.bdb \(7\x2F[1-9][0-9]+\)
-cursor-get-
bar:stepstep
bool\(true\)
qux:sum
bool\(true\)
qux:ergo
bool\(true\)
bar:stepstep
bool\(true\)
baz:jump
bool\(true\)
foo:hop
-cursor-put-
bool\(true\)
bool\(true\)
bool\(true\)
tin
kan
ton
-cursor-out-
bool\(true\)
-range-
baz
foo
qux
-keys-
bar
baz
-out-
bool\(true\)
bool\(false\)
bool\(true\)
-int-double-
bool\(true\)
bool\(true\)
int\(1980\)
float\(20\.35\)
-misc-
bool\(true\)
bool\(true\)
7\x2F[1-9][0-9]+
bool\(true\)
bool\(true\)
-arrayaccess-
0
bool\(false\)
bool\(true\)
1
Hypertext Preprocessor
0
-transaction-
0
bool\(true\)
bool\(true\)
bool\(true\)
0
bool\(true\)
bool\(true\)
bool\(true\)
1
-cleanup-
bool\(true\)
OK
