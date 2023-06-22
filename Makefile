bfc: bfc.c
	cc -std=c89 -Wall -Wextra -pedantic -o bfc bfc.c

debug: bfc.c
	cc -g -std=c89 -Wall -Wextra -pedantic -o bfc bfc.c

man: bfc
	help2man ./$(PROJECT) -n "Brainfuck compiler" -N -o $(PROJECT).1

test: test-cli test-compile
	@echo PASS

test-cli: bfc
	# Exit status.
	./bfc 2> /dev/null; [ $$? -eq 1 ]
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
	./bfc 2>&1 | grep -q 'bfc: Program filename must be specified'
	./bfc -s 2>&1 | grep -q 'bfc: Option -s requires compiler command'
	./bfc -o 2>&1 | grep -q 'bfc: Option -o requires filename or path'
	./bfc -x 2>&1 | grep -q 'bfc: Unknown option'
	./bfc x y 2>&1 | grep -q 'bfc: Surplus source filename'
	./bfc -i -c x.bf 2>&1 | grep -q 'bfc: Option -i cannot be combined'
	./bfc -i -s x x.bf 2>&1 | grep -q 'bfc: Option -i cannot be combined'
	./bfc -i -o x x.bf 2>&1 | grep -q 'bfc: Option -i cannot be combined'
	./bfc x.c 2>&1 | grep -q 'bfc: Source and intermediate filenames are same'
	./bfc x 2>&1 | grep -q 'bfc: Source and output filenames are same'
	./bfc -h | grep -q 'Usage:'
	./bfc --help | grep -q 'Usage:'
	./bfc -v | grep -q '^bfc'
	./bfc --version | grep -q '^bfc'

test-compile: bfc
	# Exit status.
	> test.bf; ./bfc test.bf
	printf '[.]' > test.bf; ./bfc test.bf
	printf '[.]]' > test.bf 2> /dev/null; ./bfc test.bf; [ $$? -eq 1 ]
	printf '[.\n.]]\n' > test.bf 2> /dev/null; ./bfc test.bf; [ $$? -eq 1 ]
	printf '[[.]\n' > test.bf 2> /dev/null; ./bfc test.bf; [ $$? -eq 1 ]
	printf '[.]' > test.bf; ./bfc -s 'echo %s %s' test.bf 2> /dev/null
	# Messages.
	printf '[.]' > test.bf; ./bfc test.bf
	printf '[.]]' > test.bf; ./bfc test.bf 2>&1 | grep -q 'bfc: Unexpected ] at line 1 col 4'
	printf '[.\n.]]\n' > test.bf; ./bfc test.bf 2>&1 | grep -q 'bfc: Unexpected ] at line 2 col 3'
	printf '[[.]\n' > test.bf; ./bfc test.bf 2>&1 | grep -q 'bfc: Unexpected end of file'
	printf '[.]' > test.bf; ./bfc -s 'echo %s %s' test.bf | grep -q '^test.c test'

test-programs: bfc
	echo '

clean:
	rm -rf latex/ html/
	rm -f bfc
