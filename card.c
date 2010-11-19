#include "jcomp.h"

struct card_source_data {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

void card_draw (data_source *source, u8 *row)
{
    int i;
    int j;
    int end;
    struct card_source_data *data;

    data = (struct card_source_data *)source->data;

    i = source->start;
    j = source->target_start * 3;
    end = source->end;
    

    while (i < end) {
        i++;
        row[j++] = data->r;
        row[j++] = data->g;
        row[j++] = data->b;
    }
}

data_source *make_card(u8 r, u8 g, u8 b,
                       ulong target_start,
                       ulong start,
                       ulong end)
{
    struct card_source_data *data;
    data_source *card;

    data = (struct card_source_data *)pmalloc(sizeof(struct card_source_data));
    card = (data_source *)pmalloc(sizeof(data_source));

    data->r = r;
    data->g = g;
    data->b = b;

    card->initialize_fun = no_op;
    card->deinitialize_fun = no_op;
    card->draw_fun = card_draw;
    card->data = data;
    card->target_start = target_start;
    card->start = start;
    card->end = end;

    return card;
}
