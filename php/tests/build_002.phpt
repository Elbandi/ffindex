--TEST--
ffindex build test: one files
--SKIPIF--
<?php if (!extension_loaded("ffindex")) die("skip"); ?>
--FILE--
<?php

$data_name = tempnam(sys_get_temp_dir(), 'test.dat');
$index_name = tempnam(sys_get_temp_dir(), 'test.idx');

var_dump(ffindex_build($data_name, $index_name, dirname(__FILE__)."/build_002.phpt"));
var_dump( file_exists($data_name) );
var_dump( file_exists($index_name) );

?>
--EXPECT--
bool(true)
bool(true)
bool(true)
