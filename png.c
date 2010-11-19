#include <stdio.h>
#include <png.h>
#include "jcomp.h"
#include "util.h"

#define PNG_MAGIC_SIZE 8

/* From Image Compositing Fundamentals, Alvy Ray Smith */
#define INT_MULT(a,b,t) ( (t) = (a) * (b) + 0x80, ( ( ( (t)>>8 ) + (t) )>>8 ) )
#define INT_LERP(p, q, a, t) ( (p) + INT_MULT( a, ( (q) - (p) ), t ) )

typedef struct _png_source_data {
    char *filename;
    char *row_data;
    FILE *fh;
    png_structp png;
    png_infop png_info;
    int rows_left;
} png_source_data;

void png_initialize (data_source *source)
{
    FILE *fh;
    png_structp png;
    png_source_data *data;

    data = (png_source_data *)source->data;

    if (!(fh = fopen(data->filename, "rb"))) {
        fail("could not open png input file");
    }

    data->png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                       NULL,
                                       NULL,
                                       NULL);

    if (!data->png) {
        fail("could not allocate png reader");
    }

    data->png_info = png_create_info_struct(data->png);

    if (!data->png_info) {
        fail("could not allocate png info structure");
    }

    png_init_io(data->png, fh);

    png_read_info(data->png, data->png_info);

    if (data->png_info->color_type != PNG_COLOR_TYPE_RGB_ALPHA) {
        fail("png is not rgbalpha");
    }

    if (data->png_info->bit_depth != 8) {
        fail("only 8 bit pngs supported");
    }

    data->row_data = pmalloc(data->png_info->width * 4);
    data->rows_left = data->png_info->height;
}

void png_draw (data_source *source, u8 *row)
{
    png_source_data *data;
    char *input;
    char *output;
    int i, j, end, temp;
    unsigned char rp, gp, bp, alpha;
    unsigned char rq, gq, bq;

    data = (png_source_data *)source->data;
    input = data->row_data;
    output = row;

    if (data->rows_left > 0) {
        data->rows_left--;

        png_read_row(data->png, (png_bytep)input, NULL);

        i = source->target_start * 3;
        j = source->start * 4;
        end = source->end * 4;

        while (j < end) {
            rq = input[j++];
            gq = input[j++];
            bq = input[j++];
            alpha = input[j++];

            rp = output[i + 0];
            gp = output[i + 1];
            bp = output[i + 2];

            output[i + 0] = INT_LERP(rp, rq, alpha, temp);
            output[i + 1] = INT_LERP(gp, gq, alpha, temp);
            output[i + 2] = INT_LERP(bp, bq, alpha, temp);

            i += 3;
        }
    }
}

data_source *make_png (char *filename,
                       unsigned long target_start,
                       unsigned long start,
                       unsigned long end)
{
    data_source *source;
    png_source_data *data;

    source = make_data_source();
    data = (png_source_data *)pmalloc(sizeof(png_source_data));

    data->filename = filename;

    source->data = data;
    source->target_start = target_start;
    source->start = start;
    source->end = end;

    source->initialize_fun = png_initialize;
    source->draw_fun = png_draw;

    /* FIXME: Really deinitialize eventually */
    source->deinitialize_fun = no_op;

    return source;
}


void png_dimensions (char *filename, ulong *width, ulong *height)
{
    FILE *fh;
    png_structp png;
    png_infop png_info;

    if (!(fh = fopen(filename, "rb"))) {
        fail("could not open png input file");
    }

    png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                 NULL,
                                 NULL,
                                 NULL);

    if (!png) {
        fail("could not allocate png reader");
    }

    png_info = png_create_info_struct(png);

    if (!png_info) {
        fail("could not allocate png info structure");
    }

    png_init_io(png, fh);

    png_read_info(png, png_info);

    if (png_info->color_type != PNG_COLOR_TYPE_RGB_ALPHA) {
        fail("png is not rgbalpha");
    }

    if (png_info->bit_depth != 8) {
        fail("only 8 bit pngs supported");
    }

    *width = png_info->width;
    *height = png_info->height;

    png_destroy_read_struct(&png, &png_info, NULL);
}

    
                      
        
    
    
