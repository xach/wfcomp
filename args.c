#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "jcomp.h"


ulong arg_read_ulong(char *start, char **end)
{
    char *s = start;
    long result = 0;

    if (*s != '+') {
        fail("parse failed looking for '+'");
    }

    s++;

    if (!*s) {
        fail("parse failed: end of string when reading number");
    }

    while (*s) {
        if (*s == '\0' || !isdigit(*s)) {
            break;
        }
        s++;
    }

    return (ulong )strtol(start, end, 10);
}
        
        
char *arg_read_dispatch(char *start, char **end)
{
    char *s;
    char *dispatch;
    int size;

    s = strchr(start, ':');

    if (s != NULL) {
        size = s - start;
        dispatch = (char *)pzmalloc(size + 1);
        memcpy(dispatch, start, size);
        *end = s + 1;
        return dispatch;
    } else {
        fail("no dispatch name found in arg");
    }
}

u8 hexvalue(int c)
{
    if (c <= '9' && '0' <= c) {
        return c - '0';
    }

    if (c <= 'F' && 'A' <= c) {
        return 10 + c - 'A';
    }

    if (c <= 'f' && 'a' <= c) {
        return 10 + c - 'a';
    }

    fail("bad hexvalue");
}

u8 hexhex(char *string)
{
    if (string[0] && string[1]) {
        return (hexvalue(string[0]) << 4) + hexvalue(string[1]);
    }

    fail("bad hexhex");
}
        
    
rgbcolor *arg_read_hexcolor(char *string, char **end)
{
    rgbcolor *color;

    color = (rgbcolor *)pmalloc(sizeof(rgbcolor));

    if (*string != '#') {
        fail("missing # when looking for hexcolor");
    }

    color->red = hexhex(string + 1);
    color->green = hexhex(string + 3);
    color->blue = hexhex(string + 5);

    *end = string + 7;
    return color;
}
     

char *arg_read_filename(char *string, char **end)
{
    struct stat *st;
    char *filename;

    if (*string != '@') {
        fail("expected '@' missing");
    }

    string++;

    filename = (char *)pzmalloc(strlen(string) + 1);
    strcpy(filename, string);

    st = (struct stat *)alloca(sizeof(struct stat));

    if (!st) {
        fail("allocating stat structure");
    }

    if (stat(filename, st) != 0) {
        fail("file stat failed");
    }

    if (end) {
        *end = string + strlen(filename);
    }

    return filename;
}


prim_source *make_prim_source()
{
    prim_source *new = (prim_source *)pmalloc(sizeof(prim_source));

    new->height = 0;
    new->width = 0;
    new->x = 0;
    new->y = 0;
    new->source = NULL;

    return new;
}

prim_source *arg_read_jpeg(char *string)
{
    prim_source *image;
    char *filename;
    ulong x, y, width, height;
    char *s = string;

    x = arg_read_ulong(s, &s);
    y = arg_read_ulong(s, &s);

    filename = arg_read_filename(s, NULL);

    jpeg_dimensions(filename, &width, &height);

    image = make_prim_source();

    image->x = x;
    image->y = y;
    image->width = width;
    image->height = height;
    image->source = make_jpeg(filename, 0, 0, 0);
    initialize(image->source);

    return image;
}

prim_source *arg_read_png(char *string)
{
    prim_source *image;
    char *filename;
    ulong x, y, width, height;
    char *s = string;

    x = arg_read_ulong(s, &s);
    y = arg_read_ulong(s, &s);

    filename = arg_read_filename(s, NULL);

    png_dimensions(filename, &width, &height);

    image = make_prim_source();

    image->x = x;
    image->y = y;
    image->width = width;
    image->height = height;
    image->source = make_png(filename, 0, 0, 0);
    initialize(image->source);

    return image;
}

/* Example card: "card:+0+0+100+50#ff3399" */

prim_source *arg_read_card(char *string)
{
    rgbcolor *color;
    prim_source *prim;
    char *s = string;

    prim = make_prim_source();

    prim->x = arg_read_ulong(s, &s);
    prim->y = arg_read_ulong(s, &s);
    prim->width = arg_read_ulong(s, &s);
    prim->height = arg_read_ulong(s, &s);

    color = arg_read_hexcolor(s, &s);

    prim->source = make_card(color->red, color->green, color->blue,
                             0, 0, 0);

    free(color);

    return prim;
}

base_prim_source *arg_read_base(char *string)
{
    base_prim_source *base;
    rgbcolor *color;
    char *s = string;

    base = (base_prim_source *)pmalloc(sizeof(base_prim_source));
    
    base->width = arg_read_ulong(s, &s);
    base->height = arg_read_ulong(s, &s);
    color = arg_read_hexcolor(s, &s);

    base->red = color->red;
    base->green = color->green;
    base->blue = color->blue;

    free(color);

    return base;
}

struct dispatch_entry {
    char *name;
    prim_source *(*dispatch_fun)(char *);
} disptab[] = {
    "jpeg", arg_read_jpeg,
    "png", arg_read_png,
    "card", arg_read_card,
    NULL, NULL
};


prim_source *invoke_dispatch(char *dispatch, char *rest)
{
    int i;

    for (i = 0; disptab[i].name; i++) {
        if (strcmp(dispatch, disptab[i].name) == 0) {
            return (disptab[i].dispatch_fun)(rest);
        }
    }
    fail("unknown dispatcher");
}



