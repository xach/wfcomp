#include "jcomp.h"

void initialize (data_source *source)
{
    (source->initialize_fun)(source);
}

void draw (data_source *source, u8 *row)
{
    (source->draw_fun)(source, row);
}

void deinitialize (data_source *source)
{
    (source->deinitialize_fun)(source);
}

void no_op (data_source *source)
{
    return;
}

data_source *make_data_source()
{
    return (data_source *)pmalloc(sizeof(data_source));
}

