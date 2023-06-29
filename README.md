Brainfuck Compiler and Interpreter
==================================

`bfc` is a Brainfuck compiler and interpreter. The compiler compiles
Brainfuck source code to C code and builds the C code into a binary
executable file. The interpreter reads Brainfuck source code and
executes it immediately without compiling it.

Brainfuck is an esoteric programming language with only eight
commands. More information on the language, its commands, and sample
programs can be found in the following Wikipedia article:
<http://en.wikipedia.org/wiki/Brainfuck>.


Get Started
-----------

If there is a C compiler available on a Unix or Linux system, the
easiest way to build and install this project is:

```sh
make
sudo make install
```

Then this project may be run as follows:

```sh
echo '++++++++[>++++++++<-]>+.[-]++++++++++.' > a.bf
bfc a.bf && ./a
bfi a.bf
```

The command `bfc` is the compiler that compiles the Brainfuck source
code into a binary executable. The command `bfi` is the interpreter
that interprets and runs the Brain program immediately. The `bfc` and
`bfi` commands above should each print the letter `A` followed by a
newline.

For usage details, enter one of the following commands:

```sh
bfc --help
bfi --help
man bfc
```

Note that the binaries `bfc` and `bfi` are identical. When `bfc` or
`bfi` is run, the running process first checks its own process name.
If it turns out to be `bfi` it runs in interpreter mode. Otherwise, it
runs in compiler mode. Also, note that `bfi` is equivalent to `bfc
-i`, so while the `bfc` command without the `-i` option runs in
compiler mode, the `bfc -i` and `bfi` commands run in interpreter
mode.


License
-------

This is free and open source software. You can use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of it,
under the terms of the MIT License. See [LICENSE.md][L] for details.

This software is provided "AS IS", WITHOUT WARRANTY OF ANY KIND,
express or implied. See [LICENSE.md][L] for details.

[L]: LICENSE.md
