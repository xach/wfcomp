#ifndef JCOMP_H
#define JCOMP_H

#include <stdio.h>
#include <jpeglib.h>


/* Type shorthand */

typedef unsigned char u8;
typedef unsigned long ulong;
typedef unsigned int uint;

typedef struct _data_source {
    void (*initialize_fun)(struct _data_source *);
    void (*draw_fun)(struct _data_source *, u8 *);
    void (*deinitialize_fun)(struct _data_source *);

    /* The first target column into which the data is drawn */
    ulong target_start;

    /* Start column (inclusive) for the source */
    ulong start;
    /* End column (exclusive) for the source */
    ulong end;

    /* Source-specific data */
    void *data;
} data_source;


/* The compositor */

typedef struct _compositor {
    char *output_filename;
    char *comment;
    ulong width;
    ulong height;

    /* The backing card */
    data_source *base;

    /* JPEG output */
    u8 *row;
    struct jpeg_compress_struct *cinfo;
    JSAMPROW row_pointer[1];

    /* Data sources per row */
    data_source ***row_sources;
    uint source_count;
} compositor;



/* Prototypes */

/* util.c */
void fail(char *);
void *pmalloc(size_t);
void *pzmalloc(size_t);

/* card.c */
data_source *make_card(u8 r, u8 g, u8 b,
                       ulong target_offset,
                       ulong start,
                       ulong end);

/* compositor.c */
compositor *make_compositor(char *filename,
                            char *comment,
                            ulong width, ulong height,
                            u8 red, u8 green, u8 blue,
                            u8 data_source_count);

/* jpeg.c */
data_source *make_jpeg(char *filename, ulong target_start,
                       ulong start, ulong end);

void initialize (data_source *);
void draw (data_source *, u8 *);
void deinitialize (data_source *);
void no_op (data_source *);
data_source *make_data_source();

void png_dimensions(char *filename, ulong *width, ulong *height);
data_source *make_png (char *filename,
                       unsigned long target_start,
                       unsigned long start,
                       unsigned long end);

/* args.c */

typedef enum _primtype {
    PRIM_CARD,
    PRIM_FILE
} primtype;
    

typedef struct _prim_source {
    primtype type;
    ulong x;
    ulong y;
    ulong width;
    ulong height;
    data_source *source;
} prim_source;


typedef struct _rgbcolor {
    u8 red;
    u8 green;
    u8 blue;
} rgbcolor;

typedef struct _base_prim_source {
    u8 red;
    u8 green;
    u8 blue;
    ulong width;
    ulong height;
} base_prim_source;

char *arg_read_dispatch(char *, char **);
prim_source *invoke_dispatch(char *dispatch, char *rest);
prim_source *arg_read_card(char *string);
base_prim_source *arg_read_base(char *string);

#endif /* JCOMP_H */
