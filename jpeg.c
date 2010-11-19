#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <jpeglib.h>
#include "jcomp.h"

struct jpeg_source_data {
    char *filename;
    char *row_data;
    JSAMPROW sample_row[1];
    FILE *fh;
    struct jpeg_decompress_struct *dcinfo;
};

void jpeg_draw_grayscale (data_source *, u8 *);

void jpeg_initialize (data_source *source)
{
    struct jpeg_source_data *data;
    struct jpeg_error_mgr *jerr;
    int component_count;

    data = (struct jpeg_source_data *)source->data;

    jerr = pmalloc(sizeof(struct jpeg_error_mgr));

    data->dcinfo = pmalloc(sizeof(struct jpeg_decompress_struct));
    data->dcinfo->err = jpeg_std_error(jerr);
    jpeg_create_decompress(data->dcinfo);

    if (!(data->fh = fopen(data->filename, "rb"))) {
        perror("open");
        exit(1);
    }

    jpeg_stdio_src(data->dcinfo, data->fh);

    jpeg_read_header(data->dcinfo, TRUE);
    jpeg_start_decompress(data->dcinfo);

    switch (data->dcinfo->out_color_components) {
    case 1:
        source->draw_fun = jpeg_draw_grayscale;
    case 3:
        break;
    default:
        fail("unable to handle jpeg with arbitrary color components");
    }

    data->row_data = (char *)pmalloc(data->dcinfo->output_width *
                                     data->dcinfo->out_color_components);
    data->sample_row[0] = data->row_data;
}

void jpeg_draw (data_source *source, u8 *row)
{
    struct jpeg_source_data *data;
    char *input;
    char *output;
    int i, j, end;

    data = (struct jpeg_source_data *)source->data;

    if (data->dcinfo->output_scanline < data->dcinfo->output_height) {
        int count = jpeg_read_scanlines(data->dcinfo, data->sample_row, 1);
        if (count != 1) {
            fail("couldn't read a jpeg row");
        }

        i = source->target_start * 3;
        j = source->start * 3;
        end = source->end * 3;

        input = data->row_data;
        output = row;

        while (j < end) {
            output[i++] = input[j++];
        }
    }
}


void jpeg_draw_grayscale (data_source *source, u8 *row)
{
    struct jpeg_source_data *data;
    char *input;
    char *output;
    int i, j, end;

    data = (struct jpeg_source_data *)source->data;

    if (data->dcinfo->output_scanline < data->dcinfo->output_height) {
        int count = jpeg_read_scanlines(data->dcinfo, data->sample_row, 1);
        if (count != 1) {
            fail("couldn't read a row");
        }

        i = source->target_start * 3;
        j = source->start;
        end = source->end;

        input = data->row_data;
        output = row;

        while (j < end) {
            output[i++] = input[j];
            output[i++] = input[j];
            output[i++] = input[j++];
        }
    }
}

data_source *make_jpeg (char *filename,
                        ulong target_start,
                        ulong start,
                        ulong end)
{
    data_source *source;
    struct jpeg_source_data *data;

    source = make_data_source();
    data = (struct jpeg_source_data *)pmalloc(sizeof(struct jpeg_source_data));

    data->filename = filename;

    source->data = data;
    source->target_start = target_start;
    source->start = start;
    source->end = end;

    source->initialize_fun = jpeg_initialize;
    source->draw_fun = jpeg_draw;

    /* FIXME: Really deinitialize eventually */
    source->deinitialize_fun = no_op;

    return source;
}

void jpeg_dimensions(char *filename, unsigned int *width, unsigned int *height)
{
    struct jpeg_decompress_struct *cinfo;
    struct jpeg_error_mgr *jerr;
    FILE *fh;

    jerr = alloca(sizeof(struct jpeg_error_mgr));
    cinfo = alloca(sizeof(struct jpeg_decompress_struct));

    cinfo->err = jpeg_std_error(jerr);
    jpeg_create_decompress(cinfo);

    if (!(fh = fopen(filename, "rb"))) {
        perror("open");
        exit(1);
    }

    jpeg_stdio_src(cinfo, fh);

    jpeg_read_header(cinfo, TRUE);

    *width = cinfo->image_width;
    *height = cinfo->image_height;

    jpeg_destroy_decompress(cinfo);
}
