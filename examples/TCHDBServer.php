<?php
extension_loaded('http') || dl('http.so') || exit(1);
extension_loaded('tokyocabinet') || dl('tokyocabinet.so') || exit(1);

class TCHDBRequestException extends RuntimeException {}

class TCHDBServer
{
    /**
     * A map of HTTP responce codes and HTTP responce status messages.
     */
    static private $statuses = array(
        100 => 'Continue',
        101 => 'Switching Protocols',
        200 => 'OK',
        201 => 'Created',
        202 => 'Accepted',
        203 => 'Non-Authoritative Information',
        204 => 'No Content',
        205 => 'Reset Content',
        206 => 'Partial Content',
        300 => 'Multiple Choices',
        301 => 'Moved Permanently',
        302 => 'Moved Temporarily',
        303 => 'See Other',
        304 => 'Not Modified',
        305 => 'Use Proxy',
        400 => 'Bad Request',
        401 => 'Unauthorized',
        402 => 'Payment Required',
        403 => 'Forbidden',
        404 => 'Not Found',
        405 => 'Method Not Allowed',
        406 => 'Not Acceptable',
        407 => 'Proxy Authentication Required',
        408 => 'Request Timed-out',
        409 => 'Conflict',
        410 => 'Gone',
        411 => 'Length Required',
        412 => 'Precondition Failed',
        413 => 'Request Entity Too Large',
        414 => 'Request-URI Too Large',
        415 => 'Unsupported Media Type',
        500 => 'Internal Server Error',
        501 => 'Not Implemented',
        502 => 'Bad Gateway',
        503 => 'Service Unavailable',
        504 => 'Gateway Time-out',
        505 => 'HTTP Version not supported',
    );

    /**
     * An instance of TCHDB.
     *
     * @var TCHDB
     */
    private $hdb;

    /**
     * A server socket resource.
     *
     * @var resource
     */
    private $sock;

    /**
     * A client socket resource.
     *
     * @var resource
     */
    private $conn;

    /**
     * A timeout value in seconds.
     *
     * @var int
     */
    private $timeoutSec = 1;

    /**
     * A timeout value in milliseconds.
     *
     * @var int
     */
    private $timeoutUsec = 0;

    /**
     * A maximum length of request entities.
     *
     * @var int
     */
    private $maxLength = -1;

    /**
     * Constructor.
     *
     * @param TCHDB $hdb
     * @param string $host
     * @param int $port
     * @param array $options
     * @return void
     * @throws RuntimeException
     */
    public function __construct(TCHDB $hdb, $host = 'localhost', $port = 8080, array $options = array())
    {
        $this->hdb = $hdb;

        if (strpos($host, '::') !== false) {
            $local_host = sprintf('tcp://[%s]:%d', $host, $port);
        } else {
            $local_host = sprintf('tcp://%s:%d', $host, $port);
        }

        $this->sock = stream_socket_server($local_host, $errno, $errstr, STREAM_SERVER_BIND | STREAM_SERVER_LISTEN);
        if (!$this->sock) {
            throw new RuntimeException($errstr, $errno);
        }

        if (isset($options['timeoutSec'])) {
            $this->timeoutSec = (int)$options['timeoutSec'];
        }
        if (isset($options['timeoutUsec'])) {
            $this->timeoutUsec = (int)$options['timeoutUsec'];
        }
        if (isset($options['maxLength'])) {
            $this->maxLength = (int)$options['maxLength'];
        }
        if ($this->maxLength <= 0) {
            $this->maxLength = PHP_INT_MAX;
        }
    }

    /**
     * Run a hash database server.
     *
     * @return void
     */
    public function run()
    {
        while ($this->conn = stream_socket_accept($this->sock, -1.0)) {
            try {
                stream_set_timeout($this->conn, $this->timeoutSec, $this->timeoutUsec);

                /**
                 * Receive and parse HTTP headers.
                 */
                $headers_string = '';
                $line = '';
                while (!feof($this->conn) && ($line = fgets($this->conn)) !== false) {
                    if (strlen(trim($line)) == 0) {
                        break;
                    }
                    $headers_string .= $line;
                }

                if ($line === false) {
                    $info = stream_get_meta_data($this->conn);
                    $code = $info['timed_out'] ? 408 : 400;
                    throw new TCHDBRequestException(self::$statuses[$code], $code);
                }

                $headers = HttpUtil::parseHeaders($headers_string);
                //print_r($headers);

                if (!$headers ||
                    !isset($headers['Request Method']) ||
                    !isset($headers['Request Url']) ||
                    !isset($headers['Host']))
                {
                    throw new TCHDBRequestException(self::$statuses[400], 400);
                }

                /**
                 * Decode an URL.
                 */
                $key = ltrim($headers['Request Url'], '/');
                if (strlen($key) == 0) {
                    throw new TCHDBRequestException(self::$statuses[400], 400);
                }
                $key = rawurldecode($key);

                /**
                 * Handle a request.
                 */
                switch (strtoupper($headers['Request Method'])) {
                  case 'GET':
                    $this->handleGet($key);
                    break;
                  case 'PUT':
                    $this->handlePut($key, $headers);
                    break;
                  case 'DELETE':
                    $this->handleDelete($key);
                    break;
                  default:
                    $this->returnStatus(405);
                }
            } catch (TCHDBRequestException $e) {
                $this->returnStatus($e->getCode(), $e->getMessage());
            } catch (Exception $e) {
                $this->returnStatus(500, $e->getMessage());
            }
            fclose($this->conn);
        }
    }

    /**
     * Handle a GET request.
     *
     * @param string $key
     * @return void
     * @throws TCException
     */
    private function handleGet($key)
    {
        if (($data = $this->hdb->get($key)) === null) {
            $this->returnStatus(404, "'$key' not found.");
        } else {
            $this->returnResponse(200, $data, 'application/octet-stream');
        }
    }

    /**
     * Handle a PUT request.
     *
     * @param string $key
     * @param array $headers
     * @return void
     * @throws TCHDBRequestException, TCException
     */
    private function handlePut($key, array $headers)
    {
        /**
         * Check for "Content-Length" header.
         */
        $length = -1;

        foreach ($headers as $name => $value) {
            if (!strcasecmp('Content-Length', $name)) {
                if (!is_string($value)) {
                    throw new TCHDBRequestException(self::$statuses[400], 400);
                }
                $length = (int)$value;
                break;
            }
        }

        if ($length < 0) {
            throw new TCHDBRequestException(self::$statuses[411], 411);
        } elseif ($length > $this->maxLength) {
            throw new TCHDBRequestException(self::$statuses[413], 413);
        }

        /**
         * Check for "Except: 100-continue" header.
         */
        foreach ($headers as $name => $value) {
            if (!strcasecmp('Expect', $name)) {
                if (!is_string($value)) {
                    throw new TCHDBRequestException(self::$statuses[400], 400);
                }
                if (stripos($value, '100-continue') !== false) {
                    $this->returnStatus(100);
                    usleep(1000);
                }
                break;
            }
        }

        /**
         * Read the request entity.
         */
        $body = '';

        while (strlen($body) < $length && !feof($this->conn)) {
            if (($buf = fread($this->conn, $length - strlen($body))) === false) {
                break;
            }
            $body .= $buf;
        }

        if (strlen($body) != $length) {
            $info = stream_get_meta_data($this->conn);
            $code = $info['timed_out'] ? 408 : 400;
            throw new TCHDBRequestException(self::$statuses[$code], $code);
        }

        /**
         * Put the request entity.
         */
        $this->hdb->put($key, $body);

        $url = 'http://' . $headers['Host'] . $headers['Request Url'];
        $this->returnResponse(201, $url, 'text/plain', array('Location' => $headers['Request Url']));
    }

    /**
     * Handle a DELETE request.
     *
     * @param string $key
     * @return void
     * @throws TCException
     */
    private function handleDelete($key)
    {
        $this->hdb->out($key);

        $this->returnStatus(200);
    }

    /**
     * Send a simple HTTP response.
     *
     * @param int $code
     * @param string $body
     * @return void
     */
    private function returnStatus($code, $body = null)
    {
        if ($body === null) {
            $body = self::$statuses[$code];
        }
        $this->returnResponse($code, $body);
    }

    /**
     * Send a HTTP response.
     *
     * @param int $code
     * @param string $mimetype
     * @param string $body
     * @param array $headers
     * @return void
     */
    private function returnResponse($code, $body = '', $mimetype = 'text/plain', array $headers = array())
    {
        $msg = new HttpMessage();
        $msg->setType(HttpMessage::TYPE_RESPONSE);
        $msg->setResponseCode($code);
        $msg->setResponseStatus(self::$statuses[$code]);
        $headers['Content-Type'] = (string)$mimetype;
        $headers['Content-Length'] = (string)strlen($body);
        $msg->setHeaders($headers);
        $msg->setBody($body);
        fwrite($this->conn, $msg->toString());
    }
}

/**
 * Run the test server.
 */
if (PHP_SAPI == 'cli' && realpath($_SERVER['SCRIPT_FILENAME']) == __FILE__) {
    $hdb = new TCHDB();
    $hdb->open('/tmp/server-test.hdb', TCHDB::OWRITER | TCHDB::OCREAT);
    $server = new TCHDBServer($hdb);
    $server->run();
}
