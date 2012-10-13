--TEST--
ffindex_get and invalid parameters
--SKIPIF--
<?php if (!extension_loaded("ffindex")) print "skip"; ?>
--FILE--
<?php

var_dump(ffindex_get());
var_dump(ffindex_get("", "", ""));
var_dump(ffindex_get("foo", "", ""));
var_dump(ffindex_get("foo", "foo", ""));
var_dump(ffindex_get("foo", "bar", ""));

echo "Done\n";
?>
--EXPECTF--
Warning: ffindex_get() expects exactly 3 parameters, 0 given in %s on line %d
NULL

Warning: ffindex_get(): data filename cannot be empty in %s on line %d
bool(false)

Warning: ffindex_get(): index filename cannot be empty in %s on line %d
bool(false)

Warning: ffindex_get(): index and data file name sould different in %s on line %d
bool(false)

Warning: ffindex_get(): filename cannot be empty in %s on line %d
bool(false)
Done
