--TEST--
ffindex get test: get file data
--SKIPIF--
<?php if (!extension_loaded("ffindex")) die("skip"); ?>
--FILE--
<?php

$data_name = tempnam(sys_get_temp_dir(), 'test.dat').posix_getpid();
$index_name = tempnam(sys_get_temp_dir(), 'test.idx').posix_getpid();

$files = array(dirname(__FILE__)."/build_001.phpt", dirname(__FILE__)."/build_002.phpt");
exec("ffindex_build $data_name $index_name ".implode(" ", $files));

$m = md5(ffindex_get($data_name, $index_name, "build_001.phpt"));
$e = md5_file(dirname(__FILE__)."/build_001.phpt");
var_dump($m == $e);

$m = md5(ffindex_get($data_name, $index_name, "build_002.phpt"));
$e = md5_file(dirname(__FILE__)."/build_002.phpt");
var_dump($m == $e);

var_dump(ffindex_get($data_name, $index_name, "nonexists"));

@unlink($data_name);
@unlink($index_name);

?>
--EXPECT--
bool(true)
bool(true)
bool(false)
