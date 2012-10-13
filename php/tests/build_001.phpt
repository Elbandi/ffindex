--TEST--
ffindex_build and invalid parameters
--SKIPIF--
<?php if (!extension_loaded("ffindex")) print "skip"; ?>
--FILE--
<?php

var_dump(ffindex_build());
var_dump(ffindex_build("", "", ""));
var_dump(ffindex_build("foo", "", ""));
var_dump(ffindex_build("foo", "foo", ""));
var_dump(ffindex_build("foo", "bar", true));

echo "Done\n";
?>
--EXPECTF--
Warning: ffindex_build() expects at least 3 parameters, 0 given in %s on line %d
NULL

Warning: ffindex_build(): data filename cannot be empty in %s on line %d
bool(false)

Warning: ffindex_build(): index filename cannot be empty in %s on line %d
bool(false)

Warning: ffindex_build(): index and data file name sould different in %s on line %d
bool(false)

Warning: ffindex_build(): files is neither a string nor an array in %s on line %d
bool(false)
Done
