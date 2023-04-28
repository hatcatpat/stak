#include "stak.h"

#include <jack/jack.h>

jack_port_t *port_in[2];
jack_port_t *port_out[2];
jack_client_t *client;

audio_t audio;

variable_t *variable_out = NULL;

int
audio_process(jack_nframes_t nframes, void *x)
{
    static jack_default_audio_sample_t *out[CHANNELS], *in[CHANNELS];
    static int i = 0;
    static uint8_t c = 0;
    (void)x;

    for(c = 0; c < CHANNELS; ++c)
        {
            out[c] = jack_port_get_buffer(port_out[c], nframes);
            in[c] = jack_port_get_buffer(port_in[c], nframes);
        }

    if(variable_out == NULL)
        {
            for(i = 0; i < nframes; ++i)
                {
                    for(c = 0; c < CHANNELS; ++c)
                        audio.in[c] = 0;

                    variable_ll_process(&variables[0]);

                    for(c = 0; c < CHANNELS; ++c)
                        *out[c]++ = 0;
                }
        }
    else
        {
            for(i = 0; i < nframes; ++i)
                {
                    for(c = 0; c < CHANNELS; ++c)
                        audio.in[c] = *in[c]++;

                    variable_ll_process(&variables[0]);

                    for(c = 0; c < CHANNELS; ++c)
                        *out[c]++ = stack_pop_number(&variable_out->stack);
                }
        }

    if(reload)
        {
            variable_ll_merge(&variables[0], &variables[1]);
            variable_out = variable_ll_find(&variables[0], "out");
            reload = 0;
        }

    return 0;
}

int
audio_init()
{
    int c = 0;
    const char **ports;
    const char *client_name = "stak";
    const char *input_names[CHANNELS] = { "input_left", "input_right" };
    const char *output_names[CHANNELS] = { "output_left", "output_right" };
    const char *server_name = NULL;
    jack_options_t options = JackNullOption;
    jack_status_t status;

    client = jack_client_open(client_name, options, &status, server_name);

    if(client == NULL)
        {
            fprintf(stderr, "jack_client_open() failed, status = 0x%2.0x\n", status);
            if (status & JackServerFailed)
                fprintf(stderr, "Unable to connect to JACK server\n");

            return 1;
        }

    if (status & JackServerStarted)
        fprintf(stderr, "JACK server started\n");

    if (status & JackNameNotUnique)
        {
            client_name = jack_get_client_name(client);
            fprintf(stderr, "unique name: %s assigned\n", client_name);
        }

    jack_set_process_callback(client, audio_process, 0);

    audio.rate = jack_get_sample_rate(client);

    for(c = 0; c < CHANNELS; ++c)
        {
            port_in[c] = jack_port_register(client, input_names[c], JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
            port_out[c] = jack_port_register(client, output_names[c], JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

            if ((port_in[c] == NULL) || (port_out[c] == NULL))
                {
                    fprintf(stderr, "no more JACK ports available\n");
                    return 1;
                }
        }

    if (jack_activate(client))
        {
            fprintf(stderr, "cannot activate client");
            return 1;
        }

    ports = jack_get_ports(client, NULL, NULL, JackPortIsPhysical | JackPortIsOutput);
    if (ports == NULL)
        {
            fprintf(stderr, "no physical capture ports\n");
            return 1;
        }
    for(c = 0; c < CHANNELS; ++c)
        if (jack_connect(client, ports[c], jack_port_name(port_in[c])))
            fprintf(stderr, "cannot connect input ports\n");
    free(ports);

    ports = jack_get_ports(client, NULL, NULL, JackPortIsPhysical | JackPortIsInput);
    if (ports == NULL)
        {
            fprintf(stderr, "no physical playback ports\n");
            return 1;
        }
    for(c = 0; c < CHANNELS; ++c)
        if (jack_connect(client, jack_port_name(port_out[c]), ports[c]))
            fprintf(stderr, "cannot connect output ports\n");
    free(ports);

    return 0;
}

void
audio_deinit()
{
    jack_client_close(client);
}
