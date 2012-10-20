--TEST--
ffindex_list and invalid parameters
--SKIPIF--
<?php if (!extension_loaded("ffindex")) print "skip"; ?>
--FILE--
<?php

var_dump(ffindex_list());
var_dump(ffindex_list(""));

echo "Done\n";
?>
--EXPECTF--
Warning: ffindex_list() expects exactly 1 parameter, 0 given in %s on line %d
NULL

Warning: ffindex_list(): index filename cannot be empty in %s on line %d
bool(false)
Done
