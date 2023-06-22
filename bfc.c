/**
@file
@mainpage

Brainfuck compiler and interpreter.

The MIT License (MIT)
---------------------

Copyright (c) 2008-2023 Susam Pal

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>


/** Version of the program. */
#define VERSION "0.2.0-dev"

/** Author of the program. */
#define AUTHOR "Susam Pal"

/** Copyright notice. */
#define COPYRIGHT "Copyright (c) 2008-2023 " AUTHOR

/** URL to a copy of the license. */
#define LICENSE_URL "<https://susam.github.io/licenses/mit.html>"

/** URL to report issues to. */
#define SUPPORT_URL "<https://github.com/susam/bfc/issues>"

/** Maximum length of internal buffers. */
#define MAX_BUF_LEN 64

/** Maximum length of compiler command. */
#define MAX_CMD_LEN 256


/**
Return name of the leaf directory or file in the specified path.

Both backslash and forward slash are treated as path separators. A
pointer to the beginning of the substring between the last slash
(exclusive) and the end of the string is returned as the basename.
Therefore if the specified path ends with a slash then an empty string
is returned.

@param path Path string.

@return Name of the leaf directory or file in the specified path.
*/
const char *basename(const char *path)
{
    const char *base;
    if ((base = strchr(path, '\\')) != NULL) {
        return ++base;
    } else if ((base = strchr(path, '/')) != NULL) {
        return ++base;
    } else {
        return path;
    }
}


/**
Copy null-terminated byte string into a character array.

@param dst Pointer to character array to copy the string to.
@param src Pointer to null-terminated byte string to copy.
@param count Maximum number of characters to copy.

@return Destination string `dst`.
*/
char *strcp(char *dst, const char *src, size_t count)
{
    dst[0] = '\0';
    strncat(dst, src, count - 1);
    return dst;
}


/**
Check if two null-terminated byte strings are equal.

@param a Pointer to null-terminated byte string. (type: const char *)
@param b Pointer to null-terminated byte string. (type: const char *)

@return 1 if the two strings are equal; 0 otherwise.
*/
#define streq(a, b) (strcmp(a, b) == 0)


/**
Copy null-terminated byte string into a character array.

@param a Pointer to null-terminated byte string. (type: char *)
@param b Pointer to null-terminated byte string. (type: const char *)

@return Destination byte string `a`.
*/
#define strcpz(a, b) (strcp(a, b, MAX_BUF_LEN))


/**
Values returned by a function to indicate success or failure.

These return codes may be returned by a function to indicate success or
failure of its operation as well as the next course of action.
*/
enum result {
    GOOD, /**< Successful operation; program should continue. */
    EXIT, /**< Successful operation; program should exit normally. */
    FAIL  /**< Failed operation; program should exit with error. */
};


/** Global metadata of the program. */
struct meta {
    char name[MAX_BUF_LEN];      /**< Program name. */
    char debug;                  /**< Whether verbose mode is enabled. */
    char compile;                /**< Whether to compile only. */
    char compiler[MAX_BUF_LEN];  /**< Compiler command. */
    char interpret;              /**< Whether interpreter mode is enabled. */
    char src[MAX_BUF_LEN];       /**< Source filename. */
    char icc[MAX_BUF_LEN + 2];   /**< Intermediate C code filename. */
    char out[MAX_BUF_LEN];       /**< Output filename. */
    char error[MAX_BUF_LEN];     /**< Error message. */
} my;


/**
Show usage and help details of this program.
*/
void show_help(void)
{
    const char *usage =
        "Usage: %s [-d] [-c] [-i] [-o FILE] FILE\n\n";

    const char *summary =
        "Compile or interpret a Brainfuck program.\n\n";

    const char *options =
        "Options:\n"
        "  -c             Compile to C only; do not create executable.\n"
        "  -d             Compile or interpret program verbosely.\n"
        "  -o FILE        Write compiled executable to FILE.\n"
        "  -s COMMAND     Command to compile generated C source.\n"
        "  -i             Interpret program; do not compile.\n"
        "  -h, --help     Show this help message and exit.\n"
        "  -v, --version  Show version and exit.\n\n";

    const char *footer =
        "Report bugs to " SUPPORT_URL ".\n";

    printf(usage, my.name);
    printf("%s", summary);
    printf("%s", options);
    printf("%s", footer);
}


/**
 * Show version and copyright details of this program.
 */
void show_version(void)
{
    const char *s =
        "%s " VERSION "\n"
        COPYRIGHT "\n\n"

        "This is free and open source software. You can use, copy, modify,\n"
        "merge, publish, distribute, sublicense, and/or sell copies of it,\n"
        "under the terms of the MIT License. You can obtain a copy of the\n"
        "MIT License at " LICENSE_URL ".\n\n"

        "This software is provided \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
        "express or implied. See the MIT License for details.\n";

    printf(s, my.name);
}


/**
Output message on the standard error stream.

@param prefix Prefix string to add to the message.
@param format Format string for printf.
@param ...    Additional arguments.

@return EXIT_FAILURE; the caller of this function may return this code
        to indicate abnormal termination of the program.
*/
void msg(const char *prefix, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    fprintf(stderr, "%s: ", prefix);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}


/**
 * Replace extension in given name with given extension.
 *
 * @param dst Pointer to character array to write the result to.
 * @param name Filename in which to replace extension.
 * @param ext Extension.
 */
char *replace_ext(char *dst, const char *name, const char *ext) {
    char *pos;
    strcpz(dst, name);
    pos = strrchr(dst, '.');
    strcp(pos ? pos : dst + strlen(dst), ext, 3);
    return dst;
}


/**
Parse command line arguments.

@param argc Argument count
@param argv Argument vector

@return Next action to take based on whether parsing of command line
        arguments was successful or not.
*/
enum result parse_arguments(int argc, const char **argv)
{
    int i = 1;

    /* Initialize metadata. */
    strcp(my.name, basename(argv[0]), sizeof my.name);
    my.debug = 0;
    my.compile = 0;
    my.compiler[0] = '\0';
    my.interpret = 0;
    my.src[0] = '\0';
    my.icc[0] = '\0';
    my.out[0] = '\0';
    my.error[0] = '\0';

    /* Parse command line arguments. */
    while (i < argc) {
        if (streq(argv[i], "-h") || streq(argv[i], "--help")) {
            show_help();
            return EXIT;
        } else if (streq(argv[i], "-v") || streq(argv[i], "--version")) {
            show_version();
            return EXIT;
        } else if (streq(argv[i], "-d")) {
            my.debug = 1;
            ++i;
        } else if (streq(argv[i], "-c")) {
            my.compile = 1;
            ++i;
        } else if (streq(argv[i], "-s")) {
            if (i == argc - 1) {
                strcpz(my.error, "Option -s requires compiler command");
                return FAIL;
            }
            strcpz(my.compiler, argv[++i]);
            ++i;
        } else if (streq(argv[i], "-o")) {
            if (i == argc - 1) {
                strcpz(my.error, "Option -o requires filename or path");
                return FAIL;
            }
            strcpz(my.out, argv[++i]);
            ++i;
        } else if (streq(argv[i], "-i")) {
            my.interpret = 1;
            ++i;
        } else if (argv[i][0] == '-' && argv[i][1] != '\0') {
            strcpz(my.error, "Unknown option");
            return FAIL;
        } else if (my.src[0] == '\0') {
            strcpz(my.src, argv[i]);
            ++i;
        } else {
            strcpz(my.error, "Surplus source filename");
            return FAIL;
        }
    }

    if (my.debug) {
        msg("debug", "interpret: %d", my.interpret);
        msg("debug", "compile: %d", my.compile);
        msg("debug", "compiler: %s", my.compiler);
        msg("debug", "src: %s", my.src);
        msg("debug", "out: %s", my.out);
    }

    /* Validate command line arguments. */
    if (my.interpret && (my.compile || my.compiler[0] || my.out[0])) {
        strcpz(my.error, "Option -i cannot be combined with -c, -s, or -o");
        return FAIL;
    }
    if (!my.src[0]) {
        strcpz(my.error, "Program filename must be specified");
        return FAIL;
    }

    if (!my.interpret) {
        /* Apply compiler defaults. */
        if (my.compiler[0] == '\0') {
            strcpz(my.compiler, "cc -std=c89 -O2 -Wall -Wextra -pedantic %s -o %s");
        }
        if (my.icc[0] == '\0') {
            replace_ext(my.icc, my.src, ".c");
        }
        if (my.out[0] == '\0') {
            replace_ext(my.out, my.src, "");
        }

        if (my.debug) {
            msg("debug", "compiler: %s", my.compiler);
            msg("debug", "files: %s => %s => %s", my.src, my.icc, my.out);
        }

        /* Validate output filenames. */
        if (streq(my.src, my.icc)) {
            strcpz(my.error, "Source and intermediate filenames are same");
            return FAIL;
        }
        if (streq(my.src, my.out)) {
            strcpz(my.error, "Source and output filenames are same");
            return FAIL;
        }
    }

    return GOOD;
}


/**
Write a line of text to the given file.

@param file   File to write to.
@param indent Indentation level of the line.
@param line   Line to write to the file.
*/
void write_text(FILE *file, unsigned indent, const char *line) {
    unsigned i;
    for (i = 0; i < 4 * indent; ++i) {
        fprintf(file, " ");
    }
    fprintf(file, "%s", line);
}


/**
Compile source to intermediate C code.

@return Next action to take based on the result of compilation.
*/
enum result compile(void)
{
    FILE *src_file;
    FILE *icc_file;
    char ch;
    unsigned depth = 0;
    unsigned line = 1;
    unsigned col = 0;

    if ((src_file = fopen(my.src, "r")) == NULL) {
        strcpz(my.error, "Cannot open source file");
        return FAIL;
    }

    if ((icc_file = fopen(my.icc, "w")) == NULL) {
        strcpz(my.error, "Cannot open intermediate file");
        return FAIL;
    }

    write_text(icc_file, depth++, "#include <stdio.h>\n\nint main()\n{\n");
    write_text(icc_file, depth, "unsigned char cells[30000] = {0};\n");
    write_text(icc_file, depth, "unsigned char *ptr = cells;\n");
    while ((ch = fgetc(src_file)) != EOF) {
        ++col;
        switch (ch) {
        case '>':
            write_text(icc_file, depth, "++ptr;\n");
            break;
        case '<':
            write_text(icc_file, depth, "--ptr;\n");
            break;
        case '+':
            write_text(icc_file, depth, "++(*ptr);\n");
            break;
        case '-':
            write_text(icc_file, depth, "--(*ptr);\n");
            break;
        case '.':
            write_text(icc_file, depth, "putchar(*ptr);\n");
            break;
        case '[':
            write_text(icc_file, depth++, "while (*ptr) {\n");
            break;
        case ']':
            if (depth <= 1) {
                sprintf(my.error, "Unexpected ] at line %d col %d", line, col);
                fclose(icc_file);
                return FAIL;
            }
            write_text(icc_file, --depth, "}\n");
            break;
        case '\n':
            ++line;
            col = 0;
            break;
        }
    }
    if (depth != 1) {
        strcpz(my.error, "Unexpected end of file");
        fclose(icc_file);
        return FAIL;
    }
    write_text(icc_file, --depth, "}\n");
    fclose(icc_file);
    return GOOD;
}


/**
Compile intermediate C code to executable file.

@return Next action to take based on the result of compilation.
*/
enum result build(void)
{
    char cmd[MAX_CMD_LEN];
    int ret1;
    int ret2;
    sprintf(cmd, my.compiler, my.icc, my.out);
    ret1 = system(cmd);
    ret2 = remove(my.icc);
    if (my.debug) {
        msg("debug", "system() returned %d", ret1);
        msg("debug", "remove() returned %d", ret2);
    }
    return GOOD;
}


/**
Interpret source code and run.

@return Next action to take.
*/
enum result interpret(void)
{
    strcpz(my.error, "Interpreter BFI will soon be merged into this project");
    return FAIL;
}


/**
Start the program.

@param argc Argument count.
@param argv Argument vector.

@return EXIT_SUCCESS if the program terminates normally;
        EXIT_FAILURE if an error occurs.
*/
int main(int argc, const char **argv)
{
    enum result ret;
    if ((ret = parse_arguments(argc, argv)) == FAIL) {
        msg(my.name, my.error);
        return EXIT_FAILURE;
    } else if (ret == EXIT) {
        return EXIT_SUCCESS;
    }

    if (my.interpret) {
        if ((ret = interpret()) == FAIL) {
            msg(my.name, my.error);
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if ((ret = compile()) == FAIL) {
        msg(my.name, my.error);
        return EXIT_FAILURE;
    }

    if ((ret = build()) == FAIL) {
        msg(my.name, my.error);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
