<?php
extension_loaded('http') || dl('http.so') || exit(1);

$base_url = 'http://localhost:8080/';
$key = 'hoge';
$url = $base_url . $key;
$data = 'Tokyo Cabinet';

function response_dump(HttpMessage $res)
{
    printf("Code: %d\n", $res->getResponseCode());
    printf("Body: %s\n", $res->getBody());
}

echo "GET:\n";
$req = new HttpRequest($url, HttpRequest::METH_GET);
$res = $req->send();
response_dump($res);
echo "\n";

echo "PUT:\n";
$req = new HttpRequest($url, HttpRequest::METH_PUT);
$req->setPutData($data);
//$req->setPutFile(__FILE__);
$res = $req->send();
if ($res->getResponseCode() == 100) {
    $res = $req->send();
}
response_dump($res);
echo "\n";

echo "GET:\n";
$req = new HttpRequest($url, HttpRequest::METH_GET);
$res = $req->send();
response_dump($res);
echo "\n";

echo "DELETE:\n";
$req = new HttpRequest($url, HttpRequest::METH_DELETE);
$res = $req->send();
response_dump($res);
echo "\n";

echo "GET:\n";
$req = new HttpRequest($url, HttpRequest::METH_GET);
$res = $req->send();
response_dump($res);
echo "\n";
