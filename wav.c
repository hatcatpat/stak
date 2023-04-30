#include "stak.h"

typedef struct _wav
{
    uint8_t riff[4];
    uint32_t total_size;
    uint8_t file_type[4];
    uint8_t format[4];
    uint32_t format_size;
    uint16_t type;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t align;
    uint16_t bits_per_sample;
    uint8_t data[4];
    uint32_t data_size;
} wav_t;

void
wav_print(wav_t *wav)
{
    printf("riff: %c%c%c%c\n", wav->riff[0], wav->riff[1], wav->riff[2], wav->riff[3]);
    printf("total_size: %i\n", wav->total_size);
    printf("file_type: %c%c%c%c\n", wav->file_type[0], wav->file_type[1], wav->file_type[2], wav->file_type[3]);
    printf("format: %c%c%c%c\n", wav->format[0], wav->format[1], wav->format[2], wav->format[3]);
    printf("type: %i\n", wav->type);
    printf("channels: %i\n", wav->channels);
    printf("sample_rate: %i\n", wav->sample_rate);
    printf("byte_rate: %i\n", wav->byte_rate);
    printf("align: %i\n", wav->align);
    printf("bits_per_sample: %i\n", wav->bits_per_sample);
    printf("data: %c%c%c%c\n", wav->data[0], wav->data[1], wav->data[2], wav->data[3]);
    printf("data_size: %i\n", wav->data_size);
}

buffer_t
wav_decode(char *file)
{
    buffer_t buffer;
    wav_t wav;
    FILE *f = fopen(file, "rb");

    memset(&buffer, 0, sizeof(buffer_t));

    if(f == NULL)
        return buffer;

    fread(&wav.riff, sizeof(wav_t), 1, f);

    printf("[decoded %s]\n", file);
    wav_print(&wav);

    {
        int i;
        uint_t num = wav.data_size / (wav.bits_per_sample / 8);
        int16_t *data = calloc(num, sizeof(int16_t));

        fread(data, sizeof(int16_t), num, f);
        printf("number of samples = %i\n", num);

        buffer.data = calloc(num, sizeof(float));
        buffer.chans = wav.channels;
        buffer.len = num / wav.channels;

        for(i = 0; i < num; ++i)
            buffer.data[i] = (float)data[i] / INT16_MAX;

        free(data);
    }

    fclose(f);

    return buffer;
}
