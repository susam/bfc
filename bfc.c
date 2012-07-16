/* Brainfuck compiler */

/*
 * Copyright (c) 2008 Susam Pal
 * All rights reserved.
 * 
 * Copyright (c) 2008 Susam Pal
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *     1. Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *     2. Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials provided
 *        with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define VERSION     "0.1"
#define COPYRIGHT   "Copyright (c) 2008 Susam Pal"
#define AUTHOR      "Susam Pal"
#define LICENSE \
  "This is free software. You are permitted to redistribute and use it in\n" \
  "source and binary forms, with or without modification, under the terms\n" \
  "of the Simplified BSD License. See <http://susam.in/licenses/bsd/> for\n" \
  "the complete license."

#define STACK_SIZE          1000  /* Default loop stack size */
#define STACK_GROWTH_FACTOR 0.1   /* How much stack to add when its full */

void compile(char *asm_filename, char *src_filename);
char *string(char *str);
char *replace_extension(char *name, char *ext);
void help();
void version();

enum stage {
    COMPILE,   /* Compile only */
    ASSEMBLE,  /* Compile and assemble only */
    LINK       /* Compile, assemble and link */
};

struct info_t {
    char *pname;         /* Process name */
    char *ifilename;     /* Input source code file name */
    char *ofilename;     /* Output file name */
    enum stage ostage;   /* Final stage that generates the output file */
    char *arr_size;      /* Memory allocated for the executable */
} info;

/*
 * Parses the command line, sets the compile options, invokes the
 * functions and commands necessary to generate the output file.
 */
int main(int argc, char **argv)
{
    int verbose = 0;               /* 1 enables verbosity; 0 disables */
    char *arr_size = "30000";      /* Default size of array of cells */
    char *asm_filename;            /* File name for assembly code */
    char *obj_filename;            /* File name for object code */
    char *exe_filename = "a.out";  /* File name for executable code */
    char *command;                 /* Buffer for command line strings */
    size_t i;                      /* Counter */
    size_t len;                    /* Stores string lengths */

    /* Set default compile options */
    if ((info.pname = strrchr(argv[0], '/')) == NULL) {
        info.pname = argv[0];
    } else {
        info.pname++; /* Address of the basename part in argv[0] */
    }
    info.ifilename = NULL;
    info.ofilename = NULL;
    info.ostage = LINK; 
    info.arr_size = arr_size;

    /* Parse command line and set compile options */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            info.arr_size = argv[++i];
        } else if (strcmp(argv[i], "-S") == 0 && info.ostage > COMPILE) {
            info.ostage = COMPILE;
        } else if (strcmp(argv[i], "-c") == 0 && info.ostage > ASSEMBLE) {
            info.ostage = ASSEMBLE;
        } else if (strcmp(argv[i], "-o") == 0) {
            info.ofilename = argv[++i];
        } else if (strcmp(argv[i], "-v") == 0 ||
                   strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
        } else if (strcmp(argv[i], "--help") == 0 ||
                   strcmp(argv[i], "-h") == 0) {
            help();
            exit(EXIT_SUCCESS);
        } else if (strcmp(argv[i], "-V") == 0 ||
                   strcmp(argv[i], "--version") == 0) {
            version();
            exit(EXIT_SUCCESS);
        } else {
            info.ifilename = argv[i];
        }
    }

    /* If input source code file name is not specified, exit */
    if (info.ifilename == NULL) {
        fprintf(stderr, "%s: No input files\n", info.pname);
        exit(EXIT_FAILURE);
    }

    /*
     * Phase 1: Compile
     */

    /* Determine name for assembly code filename */
    if (info.ostage == COMPILE && info.ofilename != NULL) {
        asm_filename = string(info.ofilename);
    } else {
        asm_filename = replace_extension(info.ifilename, "s"); 
    }

    /* Compile the source file into assembly file */
    if (verbose) {
        printf("Compiling: compile(\"%s\", \"%s\")\n",
               asm_filename, info.ifilename);
    }
    compile(asm_filename, info.ifilename);

    /* If compile only option was specified, exit */
    if (info.ostage == COMPILE) {
        free(asm_filename);
        exit(EXIT_SUCCESS);
    }

    /*
     * Phase 2: Assemble
     */

    /* Determine name for object code filename */
    if (info.ostage == ASSEMBLE && info.ofilename != NULL) {
        obj_filename = string(info.ofilename);
    } else {
        obj_filename = replace_extension(info.ifilename, "o"); 
    }

    /* Prepare command line for GNU as */
    len = strlen("as -o") + strlen(asm_filename) +
          strlen(obj_filename) + 2;
    if ((command = malloc(len)) == NULL) {
        fprintf(stderr, "%s: Out of memory while assembling", info.pname);
    }
    sprintf(command, "as -o %s %s", obj_filename, asm_filename);

    /* Assemble the assembly code into object code */
    if (verbose) {
        printf("Assembling: %s\n", command);
    }
    system(command);
    free(command);

    /* Assembly code file is not required after assembling */
    unlink(asm_filename);
    free(asm_filename);

    /* If compile and assemble only option was specified, exit */
    if (info.ostage == ASSEMBLE) {
        free(obj_filename);
        exit(EXIT_SUCCESS);
    }

    /*
     * Phase 3: Link
     */

    /* Determine name for executable code filename */
    if (info.ostage == LINK && info.ofilename != NULL) {
        exe_filename = info.ofilename;
    }

    /* Prepare command line for GNU ld */
    len = strlen("ld -o") + strlen(obj_filename) +
          strlen(exe_filename) + 2;
    if ((command = malloc(len)) == NULL) {
        fprintf(stderr, "%s: Out of memory while compiling", info.pname);
    }
    sprintf(command, "ld -o %s %s", exe_filename, obj_filename);

    /* Link the object code to executable code */
    if (verbose) {
        printf("Linking: %s\n", command);
    }
    system(command);
    free(command);

    /* Object code file is not required after linking */
    unlink(obj_filename);
    free(obj_filename);
    
    exit(EXIT_SUCCESS);
}

/*
 * Copies the string pointed to by str into a new location and returns
 * the address of the new location where the string has been copied. The
 * caller of this function must free the pointer returned by this string
 * when the string is no longer required.
 */
char *string(char *str) {
    char *new_str;
    if ((new_str = malloc(strlen(str) + 1)) == NULL) {
        fprintf(stderr, "%s: Out of memory while allocating memory for "
                        "string: %s\n", info.pname, str);
        exit(1);
    }
    strcpy(new_str, str);
    return new_str;
}

/*
 * Constructs a new string by replacing the extension name of the
 * filename pointed to by name with the extension name pointed to by
 * ext. If the filename has no extension name, the specified extension
 * name is appended to the filename. It returns the address of the new
 * string that has been constructed.
 */
char *replace_extension(char *name, char *ext) {
    char *dot = strrchr(name, '.');
    char *new_name;
    size_t len = dot == NULL ? strlen(name) : dot - name;

    if ((new_name = malloc(len + strlen(ext) + 2)) == NULL) {
        fprintf(stderr, "%s: Out of memory while changing extension of "
                        "%s to %s\n", info.pname, name, ext);
        exit(1);
    }

    strncpy(new_name, name, len);
    new_name[len] = '\0';
    strcat(new_name, ".");
    strcat(new_name, ext);
    return new_name;
}

/*
 * Compiles the brainfuck source code present in src_filename into
 * assembly code in asm_filename.
 */
void compile(char *asm_filename, char *src_filename) {

    FILE *src;                      /* Source code file */
    FILE *as;                       /* Assembly code file */
    size_t *stack;                  /* Loop stack */
    size_t top = 0;                 /* Next free location in stack */
    size_t stack_size = STACK_SIZE; /* Stack size */
    size_t loop = 0;                /* Used to generate loop labels */
    int c;

    /* Open source code file */
    if ((src = fopen(src_filename, "r")) == NULL) {
        fprintf(stderr, "%s: %s: Could not read file\n",
                info.pname, src_filename);
        exit(EXIT_FAILURE);
    }

    /* Open assembly code file */
    if ((as = fopen(asm_filename, "w")) == NULL) {
        fprintf(stderr, "%s: %s: Could not write file\n",
                info.pname, asm_filename);
        exit(EXIT_FAILURE);
    }

    /* Create loop stack */
    if ((stack = malloc(stack_size * sizeof *stack)) == NULL) {
        fprintf(stderr, "%s: Out of memory while creating loop stack "
                        "of size %lu\n", info.pname, stack_size);
        exit(EXIT_FAILURE);
    }

    /* Write assembly code */
    fprintf(as, ".section .bss\n");
    fprintf(as, "\t.lcomm buffer %s\n", info.arr_size);
    fprintf(as, ".section .text\n");
    fprintf(as, ".globl _start\n");
    fprintf(as, "_start:\n");
    fprintf(as, "\tmov $buffer, %%edi\n");
    while ((c = fgetc(src)) != EOF) {
        switch (c) {
         case '>':
            fprintf(as, "\tinc %%edi\n"); 
            break;
        case '<':
            fprintf(as, "\tdec %%edi\n");
            break;
        case '+':
            fprintf(as, "\tincb (%%edi)\n");
            break;
        case '-':
            fprintf(as, "\tdecb (%%edi)\n");
            break;
        case ',':
            fprintf(as, "\tmovl $3, %%eax\n");
            fprintf(as, "\tmovl $0, %%ebx\n");
            fprintf(as, "\tmovl %%edi, %%ecx\n");
            fprintf(as, "\tmovl $1, %%edx\n");
            fprintf(as, "\tint $0x80\n");
            break;
        case '.':
            fprintf(as, "\tmovl $4, %%eax\n");
            fprintf(as, "\tmovl $1, %%ebx\n");
            fprintf(as, "\tmovl %%edi, %%ecx\n");
            fprintf(as, "\tmovl $1, %%edx\n");
            fprintf(as, "\tint $0x80\n");
            break;
        case '[':
            if (top == stack_size) {
                stack_size *= 1 + STACK_GROWTH_FACTOR;
                if ((stack = realloc(stack,
                                     sizeof *stack * stack_size)) == NULL) {
                    fprintf(stderr, "%s: Out of memory while increasing "
                                    "loop stack to size: %lu\n",
                            info.pname, stack_size);
                    exit(EXIT_FAILURE);
                }
            }
            stack[top++] = ++loop;
            fprintf(as, "\tcmpb $0, (%%edi)\n");
            fprintf(as, "\tjz .LE%u\n", loop);
            fprintf(as, ".LB%u:\n", loop);
            break;
        case ']':
            fprintf(as, "\tcmpb $0, (%%edi)\n");
            fprintf(as, "\tjnz .LB%u\n", stack[--top]);
            fprintf(as, ".LE%u:\n", stack[top]);
            break;
        }
    }
    fprintf(as, "movl $1, %%eax\n");
    fprintf(as, "movl $0, %%ebx\n");
    fprintf(as, "int $0x80\n");

    /* Close open files */
    fclose(as);
    fclose(src);
}

/*
 * Displays help.
 */
void help()
{
    printf("Usage: %s [OPTION] ... FILE\n\n", info.pname);
    printf("Options:\n");
    printf("  " "-S             "
           "Compile only; do not assemble or link\n");
    printf("  " "-c             "
           "Compile and assemble, but do not link\n");
    printf("  " "-o FILE        "
           "Place the output into FILE\n");
    printf("  " "-s SIZE        "
           "Size of the array of byte cells\n");
    printf("  " "-v, --verbose  "
           "Display functions and commands invoked\n");
    printf("  " "-h, --help     "
           "Display this help and exit\n");
    printf("  " "-V, --version  "
           "Output version information and exit\n");
    printf("\n");
    printf("Report bugs to <susam@susam.in>.\n");
}

/*
 * Displays version and copyright details.
 */
void version()
{
    printf("%s " VERSION "\n", info.pname);
    printf(COPYRIGHT "\n\n");
    printf(LICENSE "\n\n");
    printf("Written by " AUTHOR ".\n");
}
