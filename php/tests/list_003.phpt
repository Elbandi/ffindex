--TEST--
ffindex_list test: open_basedir
--SKIPIF--
<?php if (!extension_loaded("ffindex")) die("skip"); ?>
--INI--
open_basedir=.
--FILE--
<?php

var_dump(ffindex_list("../foo"));

?>
--EXPECTF--
Warning: ffindex_list(): open_basedir restriction in effect. File(../foo) is not within the allowed path(s): (.) in %s on line %d

Warning: ffindex_list(): Unable to open '%s' for reading in %s on line %d
bool(false)
