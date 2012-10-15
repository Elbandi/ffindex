--TEST--
ffindex_build test: open_basedir
--SKIPIF--
<?php if (!extension_loaded("ffindex")) die("skip"); ?>
--INI--
open_basedir=.
--FILE--
<?php

var_dump(ffindex_build("../foo", "./bar", dirname(__FILE__)."/build_002.phpt"));
var_dump(ffindex_build("./foo", "../bar", dirname(__FILE__)."/build_002.phpt"));
var_dump(ffindex_build("./foo", "./bar", "../build_002.phpt"));
var_dump(ffindex_build("./foo", "./bar", array("../build_001.phpt", "../build_002.phpt")));

?>
--EXPECTF--
Warning: ffindex_build(): open_basedir restriction in effect. File(../foo) is not within the allowed path(s): (.) in %s on line %d

Warning: ffindex_build(): Unable to open '%s' for writing in %s on line %d
bool(false)

Warning: ffindex_build(): open_basedir restriction in effect. File(../bar) is not within the allowed path(s): (.) in %s on line %d

Warning: ffindex_build(): Unable to open '%s' for writing in %s on line %d
bool(false)

Warning: ffindex_build(): open_basedir restriction in effect. File(../build_002.phpt) is not within the allowed path(s): (.) in %s on line %d
bool(false)

Warning: ffindex_build(): open_basedir restriction in effect. File(../build_001.phpt) is not within the allowed path(s): (.) in %s on line %d
bool(false)
