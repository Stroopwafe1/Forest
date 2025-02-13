#!/usr/bin/env elvish

use path;

fn compile-compiler {
	tmp pwd = (path:abs build);
	ninja;
	./Forest ../Self-Host/.;
}

fn compile-programme {
	./Self-Host/build/Forest ./Testing/self-host-test.tree;
}

if (<= (count $args) 1) {
	compile-compiler;
	compile-programme;
} else {
	if (==s $args[0] "compiler") {
		compile-compiler;
	} elif (==s $args[0] "programme") {
		compile-programme;
	} else {
		compile-compiler;
		compile-programme;
	}
}
