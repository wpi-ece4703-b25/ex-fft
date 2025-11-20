#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <complex.h>

#include <jack/jack.h>
#include <liquid.h>

fftplan fp;

// hardcoded to 1024-point ffts
// one jack frame holds 1024 samples
// therefore, each jack frame fits one fft
#define FFTSIZE 1024
float complex input[FFTSIZE];
float complex output[FFTSIZE];

void fft_init() {
  fp = fft_create_plan(FFTSIZE, input, output, LIQUID_FFT_FORWARD, 0);
  fft_print_plan(fp);
}

jack_port_t *input_port_left;
jack_port_t *output_port_left;
jack_port_t *input_port_right;
jack_port_t *output_port_right;
jack_client_t *client;

int process (jack_nframes_t nframes, void *arg) {
  jack_default_audio_sample_t *inleft, *outleft;
  jack_default_audio_sample_t *inright, *outright;

  inleft   = jack_port_get_buffer (input_port_left, nframes);
  outleft  = jack_port_get_buffer (output_port_left, nframes);
  inright  = jack_port_get_buffer (input_port_right, nframes);
  outright = jack_port_get_buffer (output_port_right, nframes);

  for (int i=0; i<nframes; i++)
    input[i] = 0.05 * inleft[i];

  fft_execute(fp);

  for (int i=0; i<nframes; i++) {
    outleft[i]  = cabsf(output[i]);
    outright[i] = (i == 0) ? 2.0 : (i == 512) ? -2.0 : inleft[i] - 2; // create a sync
  }

  
  return 0;
}

void jack_shutdown (void *arg) {
        exit (1);
}

int main (int argc, char *argv[]) {
  const char **ports;
  const char *client_name = "simple";
  const char *server_name = NULL;
  jack_options_t options = JackNullOption;
  jack_status_t status;

  client = jack_client_open (client_name, options, &status, server_name);
  if (client == NULL) {
    fprintf (stderr, "jack_client_open() failed, status = 0x%2.0x\n", status);
    if (status & JackServerFailed) {
      fprintf (stderr, "Unable to connect to JACK server\n");
    }
    exit (1);
  }
  if (status & JackServerStarted) {
    fprintf (stderr, "JACK server started\n");
  }
  if (status & JackNameNotUnique) {
    client_name = jack_get_client_name(client);
    fprintf (stderr, "unique name `%s' assigned\n", client_name);
  }

  jack_set_process_callback (client, process, 0);
  jack_on_shutdown (client, jack_shutdown, 0);

  fft_init();
  printf ("engine sample rate: %" PRIu32 "\n", jack_get_sample_rate (client));

  input_port_left = jack_port_register (client, "input_left",
                                   JACK_DEFAULT_AUDIO_TYPE,
                                   JackPortIsInput, 0);
  output_port_left = jack_port_register (client, "output_left",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput, 0);
  input_port_right = jack_port_register (client, "input_right",
                                   JACK_DEFAULT_AUDIO_TYPE,
                                   JackPortIsInput, 0);
  output_port_right = jack_port_register (client, "output_right",
                                    JACK_DEFAULT_AUDIO_TYPE,
                                    JackPortIsOutput, 0);

  if ((input_port_left == NULL) || (output_port_left == NULL) ||
      (input_port_right == NULL) || (output_port_right == NULL)) {
    fprintf(stderr, "no more JACK ports available\n");
    exit (1);
  }

  if (jack_activate (client)) {
    fprintf (stderr, "cannot activate client");
    exit (1);
  }

  while (1)
    sleep(10);

  jack_client_close (client);
  
  exit (0);
}
