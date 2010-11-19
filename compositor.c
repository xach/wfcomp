#include <stdio.h>
#include <jpeglib.h>
#include <string.h>
#include "jcomp.h"

compositor *make_compositor(char *filename,
                            char *comment,
                            ulong width,
                            ulong height,
                            u8 red,
                            u8 green,
                            u8 blue,
                            u8 data_source_count)
{
    compositor *new;
    FILE *outfile;
    struct jpeg_error_mgr *jerr;
    int i;

    if (!(outfile = fopen(filename, "wb"))) {
        fail("fopen compositor outfile");
    }

    new = (compositor *)pmalloc(sizeof(compositor));

    /* Initialize the output JPEG structures */

    new->cinfo = (struct jpeg_compress_struct *)
        pmalloc(sizeof(struct jpeg_compress_struct));

    jerr = (struct jpeg_error_mgr *)
        pmalloc(sizeof(struct jpeg_error_mgr));

    new->cinfo->err = jpeg_std_error(jerr);
    jpeg_create_compress(new->cinfo);

    new->cinfo->image_width = width;
    new->cinfo->image_height = height;
    new->cinfo->input_components = 3;
    new->cinfo->in_color_space = JCS_RGB;

    jpeg_stdio_dest(new->cinfo, outfile);
    jpeg_set_defaults(new->cinfo);
    jpeg_set_quality(new->cinfo, 90, TRUE);

    for (i = 0; i < 3; i++) {
        new->cinfo->comp_info[i].h_samp_factor = 1;
        new->cinfo->comp_info[i].v_samp_factor = 1;
    }

    jpeg_start_compress(new->cinfo, TRUE);


    /* The rest */


    if (comment != NULL) {
        jpeg_write_marker(new->cinfo, JPEG_COM, comment, 
                          strlen(comment));
    }

    new->width = width;
    new->height = height;
    new->source_count = data_source_count;

    new->row = (u8 *)pmalloc(3 * width);
    memset(new->row, 0, 3 * width);

    new->base = make_card(red, green, blue, 0, 0, width);
    initialize(new->base);

    new->row_pointer[0] = new->row;

    new->row_sources = pmalloc(sizeof(data_source **) * height);
    memset(new->row_sources, 0, sizeof(data_source **) * height);

    return new;
}
    
    
void add_source(compositor *comp, data_source *source,
                long x, long y,
                ulong width, ulong height)

{
    int i, j, ymax;

    if ((x + width < 0) || (comp->width < x) ||
        (comp->height < y) || (y + height < 0)) {
        fail("source is off the image entirely");
    }

    /* FIXME: this should not be forbidden */
    if (y < 0) {
        fail("out-of-bounds y positions not supported yet");
    }

    if (x < 0) {
        source->target_start = 0;
        source->start = abs(x);
    } else {
        source->target_start = x;
        source->start = 0;
    }

    if (comp->width < x + width) {
        source->end = comp->width - x;
    } else {
        source->end = width;
    }

    if (y + height < comp->height) {
        ymax = y + height;
    } else {
        ymax = comp->height;
    }

    for (i = y; i < ymax; i++) {
        if (comp->row_sources[i] == NULL) {
            comp->row_sources[i] = pmalloc(sizeof(data_source **) *
                                           comp->source_count);
            memset(comp->row_sources[i],
                   0, sizeof(data_source **) * comp->source_count);
            comp->row_sources[i][0] = source;
        } else {
            int j = 0;
            data_source **sources = comp->row_sources[i];
            while (sources[j] && j < comp->source_count) {
                j++;
            }
            sources[j] = source;
        }
    }
}


void compose(compositor *comp)
{
    int i;

    for (i = 0; i < comp->height; i++) {
        data_source **sourcep = comp->row_sources[i];
        draw(comp->base, comp->row);

        if (sourcep) {
            int j = 0;
            while (sourcep[j] && (j < comp->source_count)) {
                draw(sourcep[j], comp->row);
                j++;
            }
        }

        jpeg_write_scanlines(comp->cinfo, comp->row_pointer, 1);
    }

    jpeg_finish_compress(comp->cinfo);
    jpeg_destroy_compress(comp->cinfo);
}
