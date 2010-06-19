--TEST--
TCUtil::xmlbreak() and TCUtil::xmlattrs()
--SKIPIF--
<?php include 'skipif.inc'; ?>
--FILE--
<?php
ob_start();
?>
<html>
<head>
<title>Tokyo Cabinet: a modern implementation of DBM</title>
</head>
<body>
<h1>Tokyo Cabinet: a modern implementation of DBM</h1>
<p><a href="http://tokyocabinet.sourceforge.net/">http://tokyocabinet.sourceforge.net/</a></p>
</body>
</html>
<?php
$html = ob_get_clean();
foreach (TCUtil::xmlbreak($html) as $node) {
    if ($node[0] == '<') {
        $attrs = TCUtil::xmlattrs($node);
        if (count($attrs) == 1) {
            echo $node, "\n";
        } else {
            printf('<%s>:', array_shift($attrs));
            print_r($attrs);
        }
    } else {
        if (strlen($node = trim($node))) {
            echo $node, "\n";
        }
    }
}
?>
--EXPECT--
<html>
<head>
<title>
Tokyo Cabinet: a modern implementation of DBM
</title>
</head>
<body>
<h1>
Tokyo Cabinet: a modern implementation of DBM
</h1>
<p>
<a>:Array
(
    [href] => http://tokyocabinet.sourceforge.net/
)
http://tokyocabinet.sourceforge.net/
</a>
</p>
</body>
</html>
