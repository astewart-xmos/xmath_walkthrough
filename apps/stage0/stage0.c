
#include "common.h"


// The filter coefficients
extern double filter_coef[TAP_COUNT];

double filter_sample(
    double sample_history[TAP_COUNT])
{
  double acc = 0.0;

  for(int k = 0; k < TAP_COUNT; k++)
    acc += sample_history[k] * filter_coef[k];

  return acc;
}


void filter_frame(
    double frame_out[FRAME_OVERLAP],
    double frame_in[FRAME_SIZE])
{
  // We've received FRAME_OVERLAP new samples which are in frame_in[] in 
  // reverse chronological order. We need to produce FRAME_OVERLAP output
  // samples.
  for(int s = 0; s < FRAME_OVERLAP; s++){
    timer_start();
    frame_out[s] = filter_sample(&frame_in[FRAME_OVERLAP-s-1]);
    timer_stop();
  }
}


void filter_thread(
    chanend_t c_pcm_in, 
    chanend_t c_pcm_out)
{
  // This buffer stores the audio data received from the previous thread. It is
  // large enough to fit FRAME_OVERLAP samples plus TAP_COUNT historical samples
  // so the filter can be run on each of the newly received samples.
  double frame_data[FRAME_SIZE] = {0};

  // This buffer is where output samples will be placed.
  double frame_output[FRAME_OVERLAP] = {0};

  memset(frame_data, 0, sizeof(frame_data));

  // Note that we will fill the frame_data[] buffer backwards so that coefficients
  // are stored with lowest lag first. frame_output[] are stored in forward-time
  // order.

  while(1) {
    // Input samples are int32_t, but we're pretending they're double.

    // Receive FRAME_OVERLAP samples, convert them to doubles and place them
    // in the frame buffer.
    for(int k = 0; k < FRAME_OVERLAP; k++){
      const int32_t sample_in = (int32_t) chan_in_word(c_pcm_in);
      const double sample_in_dbl = ldexp(sample_in, -31);
      frame_data[FRAME_OVERLAP-k-1] = sample_in_dbl;
    }

    // Process the samples
    filter_frame(frame_output, frame_data);

    // Convert samples back to int32_t and send them to next thread.
    for(int k = 0; k < FRAME_OVERLAP; k++){
      const double sample_out_dbl = frame_output[k];
      const int32_t sample_out = round(ldexp(sample_out_dbl, 31));
      chan_out_word(c_pcm_out, sample_out);
    }

    // Shift the frame_data[] buffer up FRAME_OVERLAP samples
    memmove(&frame_data[FRAME_OVERLAP], &frame_data[0], 
            TAP_COUNT * sizeof(double));
  }
}
