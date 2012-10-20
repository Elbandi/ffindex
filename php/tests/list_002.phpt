--TEST--
ffindex_list test: get index data
--SKIPIF--
<?php if (!extension_loaded("ffindex")) die("skip"); ?>
--FILE--
<?php

$data_name = tempnam(sys_get_temp_dir(), 'test.dat').posix_getpid();
$index_name = tempnam(sys_get_temp_dir(), 'test.idx').posix_getpid();

$files = array(dirname(__FILE__)."/build_001.phpt", dirname(__FILE__)."/build_002.phpt");
exec("../src/ffindex_build $data_name $index_name ".implode(" ", $files));

var_dump(ffindex_list($index_name));

@unlink($data_name);
@unlink($index_name);

?>
--EXPECT--
array(2) {
  [0]=>
  array(3) {
    ["name"]=>
    string(14) "build_001.phpt"
    ["length"]=>
    int(809)
    ["mtime"]=>
    int(1350378639)
  }
  [1]=>
  array(3) {
    ["name"]=>
    string(14) "build_002.phpt"
    ["length"]=>
    int(442)
    ["mtime"]=>
    int(1350378639)
  }
}

