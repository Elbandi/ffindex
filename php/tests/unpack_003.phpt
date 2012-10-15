--TEST--
ffindex_unpack test: open_basedir
--SKIPIF--
<?php if (!extension_loaded("ffindex")) die("skip"); ?>
--INI--
open_basedir=.
--FILE--
<?php

var_dump(ffindex_unpack("../foo", "./bar", "build_002.phpt"));
var_dump(ffindex_unpack(__FILE__, "../bar", "build_002.phpt"));
var_dump(ffindex_unpack("build_001.phpt", "build_002.phpt", "../foo"));

?>
--EXPECTF--
Warning: ffindex_unpack(): open_basedir restriction in effect. File(../foo) is not within the allowed path(s): (.) in %s on line %d

Warning: ffindex_unpack(): Unable to open '%s' for reading in %s on line %d
bool(false)

Warning: ffindex_unpack(): open_basedir restriction in effect. File(../bar) is not within the allowed path(s): (.) in %s on line %d

Warning: ffindex_unpack(): Unable to open '%s' for reading in %s on line %d
bool(false)

Warning: ffindex_unpack(): open_basedir restriction in effect. File(../foo) is not within the allowed path(s): (.) in %s on line %d
bool(false)
