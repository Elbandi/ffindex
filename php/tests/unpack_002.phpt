--TEST--
ffindex_unpack test: unpack files to directory
--SKIPIF--
<?php if (!extension_loaded("ffindex")) die("skip"); ?>
--FILE--
<?php

$data_name = tempnam(sys_get_temp_dir(), 'test.dat').posix_getpid();
$index_name = tempnam(sys_get_temp_dir(), 'test.idx').posix_getpid();

$outdir = tempnam(sys_get_temp_dir(), 'test').posix_getpid();
var_dump(mkdir($outdir));

$files = array(dirname(__FILE__)."/build_001.phpt", dirname(__FILE__)."/build_002.phpt");
exec("../src/ffindex_build $data_name $index_name ".implode(" ", $files), $dummy, $return_var);
var_dump($return_var);

var_dump(ffindex_unpack($data_name, $index_name, $outdir));
$m = md5($outdir."/build_001.phpt");
$e = md5_file(dirname(__FILE__)."/build_001.phpt");
var_dump($m == $e);

$m = md5($outdir."/build_002.phpt");
$e = md5_file(dirname(__FILE__)."/build_002.phpt");
var_dump($m == $e);

//var_dump(ffindex_unpack($data_name, $index_name, "nonexists"));

@unlink($data_name);
@unlink($index_name);

?>
--EXPECT--
bool(true)
int(0)
int(2)
bool(false)
bool(false)

