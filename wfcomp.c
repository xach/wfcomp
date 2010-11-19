#include <stdio.h>
#include <stdlib.h>
#include "jcomp.h"

#define OUTPUT_FILENAME_ARG 1
#define BASE_ARG 2
#define ARGS_START 3

int main(int argc, char *argv[])
{
    compositor *comp;
    char *dispatch;
    char *rest;
    char *comment;
    prim_source *prim;
    base_prim_source *base;
    char *output_file;
    int args_offset = 0;
    int i;

    if (argc < ARGS_START) {
        fail("usage: wfcomp output_file base:+HH+WW#RRGGBB [@comment] [args]");
    }

    output_file = argv[OUTPUT_FILENAME_ARG];

    /* Read the base */
    dispatch = arg_read_dispatch(argv[BASE_ARG], &rest);
    if (strcmp(dispatch, "base") != 0) {
        fail("bad base arg");
    }

    base = arg_read_base(rest);

    if (argv[ARGS_START][0] == '@') {
        comment = argv[ARGS_START] + 1;
        args_offset = 1;
    } else {
        comment = "";
    }

    comp = make_compositor(output_file,
                           comment,
                           base->width, base->height,
                           base->red, base->green, base->blue,
                           argc);
    if (argc >= ARGS_START + args_offset) {
        for (i = ARGS_START + args_offset; i < argc; i++) {
            dispatch = arg_read_dispatch(argv[i], &rest);
            prim = invoke_dispatch(dispatch, rest);

            add_source(comp, prim->source,
                       prim->x, prim->y,
                       prim->width, prim->height);
        }
    } else {
        fail("usage: args dispatch");
    }

    compose(comp);
    exit(0);
}
