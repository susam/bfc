all: bfc bfi

bfc: bfc.c
	cc -g -std=c89 -Wall -Wextra -pedantic -o bfc bfc.c

bfi: bfc
	# The 'rm' command prevents macOS from killing 'bfi' ran with the
	# updated binary. See <https://apple.stackexchange.com/a/428388>
	# for details.
	rm -f bfi
	cp bfc bfi

debug: bfc.c
	cc -g -std=c89 -Wall -Wextra -pedantic -o bfc bfc.c

man: bfc
	help2man ./$(PROJECT) -n "Brainfuck compiler" -N -o $(PROJECT).1

test: test-bfc-cli test-bfi-cli test-compiler test-interpreter test-programs
	@echo PASS

test-bfc-cli: bfc
	# Exit status.
	./bfc 2> /dev/null; [ $$? -eq 1 ]
	./bfc missing.bf 2> /dev/null; [ $$? -eq 1 ]
	./bfc -s 2> /dev/null; [ $$? -eq 1 ]
	./bfc -o 2> /dev/null; [ $$? -eq 1 ]
	./bfc -x 2> /dev/null; [ $$? -eq 1 ]
	./bfc x y 2> /dev/null; [ $$? -eq 1 ]
	./bfc -i -c x.bf 2> /dev/null; [ $$? -eq 1 ]
	./bfc -i -s x x.bf 2> /dev/null; [ $$? -eq 1 ]
	./bfc -i -o x x.bf 2> /dev/null; [ $$? -eq 1 ]
	./bfc x.c 2> /dev/null; [ $$? -eq 1 ]
	./bfc x 2> /dev/null; [ $$? -eq 1 ]
	./bfc -v > /dev/null; [ $$? -eq 0 ]
	./bfc --version > /dev/null; [ $$? -eq 0 ]
	./bfc -h > /dev/null; [ $$? -eq 0 ]
	./bfc --help > /dev/null; [ $$? -eq 0 ]
	# Messages.
	./bfc 2>&1 | grep -q 'bfc: error: program filename must be specified'
	./bfc missing.bf 2>&1 | grep -q 'bfc: error: cannot open source file'
	./bfc -s 2>&1 | grep -q 'bfc: error: option -s requires compiler command'
	./bfc -o 2>&1 | grep -q 'bfc: error: option -o requires filename or path'
	./bfc -x 2>&1 | grep -q 'bfc: error: unknown option'
	./bfc x y 2>&1 | grep -q 'bfc: error: surplus source filename'
	./bfc -i -c x.bf 2>&1 | grep -q 'bfc: error: option -i cannot be combined'
	./bfc -i -s x x.bf 2>&1 | grep -q 'bfc: error: option -i cannot be combined'
	./bfc -i -o x x.bf 2>&1 | grep -q 'bfc: error: option -i cannot be combined'
	./bfc x.c 2>&1 | grep -q 'bfc: error: source and intermediate filenames are same'
	./bfc x 2>&1 | grep -q 'bfc: error: source and output filenames are same'
	./bfc -h | grep -q 'Usage:'
	./bfc --help | grep -q 'Usage:'
	./bfc -v | grep -q '^bfc'
	./bfc --version | grep -q '^bfc'

test-bfi-cli: bfi
	# Exit status.
	./bfi 2> /dev/null; [ $$? -eq 1 ]
	./bfi missing.bf 2> /dev/null; [ $$? -eq 1 ]
	./bfi -c x.bf 2> /dev/null; [ $$? -eq 1 ]
	./bfi -i -c x.bf 2> /dev/null; [ $$? -eq 1 ]
	# Messages.
	./bfi 2>&1 | grep -q 'bfi: error: program filename must be specified'
	./bfi missing.bf 2>&1 | grep -q 'bfi: error: cannot open source file'
	./bfi -c x.bf 2>&1 | grep -q 'bfi: error: option -i cannot be combined'
	./bfi -i -c x.bf 2>&1 | grep -q 'bfi: error: option -i cannot be combined'

test-compiler: bfc
	# Exit status.
	> test.bf
	./bfc test.bf
	printf '[.]' > test.bf
	./bfc test.bf
	printf '[.]]' > test.bf 2> /dev/null
	./bfc test.bf; [ $$? -eq 1 ]
	printf '[.\n.]]\n' > test.bf 2> /dev/null
	./bfc test.bf; [ $$? -eq 1 ]
	printf '[[.]\n' > test.bf 2> /dev/null
	./bfc test.bf; [ $$? -eq 1 ]
	printf '[.]' > test.bf
	./bfc -s 'echo %s %s' test.bf 2> /dev/null
	# Messages.
	printf '[.]' > test.bf
	./bfc test.bf
	printf '[.]]' > test.bf
	./bfc test.bf 2>&1 | grep -q 'bfc: error: unexpected ] at line 1 col 4'
	printf '[.\n.]]\n' > test.bf
	./bfc test.bf 2>&1 | grep -q 'bfc: error: unexpected ] at line 2 col 3'
	printf '[[.]\n' > test.bf
	./bfc test.bf 2>&1 | grep -q 'bfc: error: unexpected end of file'
	printf '[.]' > test.bf
	./bfc -s 'echo %s %s' test.bf | grep -q '^test.c test'

test-interpreter: bfi
	# Exit status.
	> test.bf
	./bfi test.bf
	printf '+[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[-]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]' > test.bf
	./bfi test.bf 2> /dev/null; [ $$? -eq 1 ]
	printf '[.]]' > test.bf
	./bfi test.bf 2> /dev/null; [ $$? -eq 1 ]
	printf '++[-]]' > test.bf
	./bfi test.bf 2> /dev/null; [ $$? -eq 1 ]
	printf '++[-\n\n]]' > test.bf
	./bfi test.bf 2> /dev/null; [ $$? -eq 1 ]
	printf '[' > test.bf
	./bfi test.bf 2> /dev/null; [ $$? -eq 1 ]
	printf '+[' > test.bf
	./bfi test.bf 2> /dev/null; [ $$? -eq 1 ]
	# Messages.
	printf '+[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[-]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]' > test.bf
	./bfi test.bf
	printf '+[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[-]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]' > test.bf
	./bfi test.bf 2>&1 | grep -q 'bfi: error: loop nesting depth exceeds 256 at line 1 col 258'
	printf '[.]]' > test.bf
	./bfi test.bf 2>&1 | grep -q 'bfi: error: unexpected ] at line 1 col 4'
	printf '++[-]]' > test.bf
	./bfi test.bf 2>&1 | grep -q 'bfi: error: unexpected ] at line 1 col 6'
	printf '++[-\n\n]]' > test.bf
	./bfi test.bf 2>&1 | grep -q 'bfi: error: unexpected ] at line 3 col 2'
	printf '++[-\n\n]]' > test.bf
	./bfi test.bf 2>&1 | grep -q 'bfi: error: unexpected ] at line 3 col 2'
	printf '[' > test.bf
	./bfi test.bf 2>&1 | grep -q 'bfi: error: unexpected end of file'
	printf '+[' > test.bf
	./bfi test.bf 2>&1 | grep -q 'bfi: error: unexpected end of file'
	printf '+[-[]' > test.bf
	./bfi test.bf 2>&1 | grep -q 'bfi: error: unexpected end of file'
	printf '+++[-[-]' > test.bf
	./bfi test.bf 2>&1 | grep -q 'bfi: error: unexpected end of file'

test-programs: bfc bfi
	make test-cat
	find examples -name "*.bf" | grep -v cat | sed 's/.bf//' | \
	while read -r name; do \
	  echo "Executing $$name.bf with compiler ..."; \
	  ./bfc "$$name.bf" && "$$name" > out.txt && \
	  diff -u out.txt "$$name.txt" || exit 1; \
	  echo "Executing $$name.bf with interpreter ..."; \
	  ./bfi "$$name.bf" > out.txt && \
	  diff -u out.txt "$$name.txt" || exit 1; \
	done

test-cat:
	./bfc examples/cat.bf
	[ "$$(printf abc | examples/cat)" = "abc" ]
	[ "$$(printf abc | ./bfi examples/cat.bf)" = "abc" ]
	[ -z "$$(examples/cat < /dev/null)" ]
	[ -z "$$(./bfi examples/cat.bf < /dev/null)" ]

clean:
	rm -rf latex/ html/
	rm -f bfc
