--TEST--
ffindex_unpack and invalid parameters
--SKIPIF--
<?php if (!extension_loaded("ffindex")) print "skip"; ?>
--FILE--
<?php

var_dump(ffindex_unpack());
var_dump(ffindex_unpack("", "", ""));
var_dump(ffindex_unpack("foo", "", ""));
var_dump(ffindex_unpack("foo", "foo", ""));
var_dump(ffindex_unpack("foo", "bar", ""));

echo "Done\n";
?>
--EXPECTF--
Warning: ffindex_unpack() expects exactly 3 parameters, 0 given in %s on line %d
NULL

Warning: ffindex_unpack(): data filename cannot be empty in %s on line %d
bool(false)

Warning: ffindex_unpack(): index filename cannot be empty in %s on line %d
bool(false)

Warning: ffindex_unpack(): index and data file name sould different in %s on line %d
bool(false)

Warning: ffindex_unpack(): outdir cannot be empty in %s on line %d
bool(false)
Done
