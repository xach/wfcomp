#ifndef JPEG_H
#define JPEG_H

#include "jcomp.h"

struct data_source *make_jpeg(char *filename,
                              unsigned long target_start,
                              unsigned long start,
                              unsigned long end);

void jpeg_dimensions(char *filename, unsigned int *width, unsigned int *height);

#endif /* JPEG_H */


    
    
