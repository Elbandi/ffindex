--TEST--
ffindex_get test: open_basedir
--SKIPIF--
<?php if (!extension_loaded("ffindex")) die("skip"); ?>
--INI--
open_basedir=.
--FILE--
<?php

var_dump(ffindex_get("../foo", "./bar", "build_002.phpt"));
var_dump(ffindex_get(__FILE__, "../bar", "build_002.phpt"));

?>
--EXPECTF--
Warning: ffindex_get(): open_basedir restriction in effect. File(../foo) is not within the allowed path(s): (.) in %s on line %d

Warning: ffindex_get(): Unable to open '%s' for reading in %s on line %d
bool(false)

Warning: ffindex_get(): open_basedir restriction in effect. File(../bar) is not within the allowed path(s): (.) in %s on line %d

Warning: ffindex_get(): Unable to open '%s' for reading in %s on line %d
bool(false)
