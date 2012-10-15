--TEST--
ffindex_build test: multiple files
--SKIPIF--
<?php if (!extension_loaded("ffindex")) die("skip"); ?>
--FILE--
<?php

$data_name = tempnam(sys_get_temp_dir(), 'test.dat');
$index_name = tempnam(sys_get_temp_dir(), 'test.idx');

$data_name2 = tempnam(sys_get_temp_dir(), 'test.dat').posix_getpid();
$index_name2 = tempnam(sys_get_temp_dir(), 'test.idx').posix_getpid();

$files = array(dirname(__FILE__)."/build_001.phpt", dirname(__FILE__)."/build_002.phpt");
var_dump(ffindex_build($data_name, $index_name, $files));
var_dump( file_exists($data_name) );
var_dump( file_exists($index_name) );

exec("../src/ffindex_build $data_name2 $index_name2 ".implode(" ", $files));
echo "Check:\n";
var_dump(md5_file($data_name) == md5_file($data_name2));
var_dump(md5_file($index_name) == md5_file($index_name2));

@unlink($data_name);
@unlink($index_name);
@unlink($data_name2);
@unlink($index_name2);

?>
--EXPECT--
bool(true)
bool(true)
bool(true)
Check:
bool(true)
bool(true)
