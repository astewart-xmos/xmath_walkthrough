
#include "common.h"

/**
 * Apply the filter to produce a single output sample.
 * 
 * In STAGE 9 this function will spawn multiple threads and compute the result
 * in parallel across those threads.
 * 
 * This function is implemented in stage9.xc so as to make use of the convenient
 * `par {}` syntax available in the XC language.
 */
q1_31 filter_sample(
    const int32_t sample_history[TAP_COUNT],
    const exponent_t history_exp,
    const headroom_t history_hr);

/**
 * Apply the filter to a frame with `FRAME_OVERLAP` new input samples, producing
 * one output sample for each new sample.
 * 
 * Computed output samples are placed into `frame_out[]` with the oldest samples
 * first (forward time order).
 * 
 * `history_in[]` contains the most recent `FRAME_SIZE` samples with the newest
 * samples first (reverse time order). The first `FRAME_OVERLAP` samples of 
 * `history_in[]` are new.
 * 
 * `history_in_exp` is the block floating-point exponent associated with the
 * samples in `history_in[]`.
 * 
 * `history_in_hr` is the headroom of the `history_in[]` vector.
 */
void filter_frame(
    q1_31 frame_out[FRAME_OVERLAP],
    const int32_t history_in[FRAME_SIZE],
    const exponent_t history_in_exp,
    const headroom_t history_in_hr)
{
  // Compute FRAME_OVERLAP output samples.
  // Each output sample will use a TAP_COUNT-sample window of the input
  // history. That window slides over 1 element for each output sample.
  // A timer (100 MHz freqency) is used to measure how long each output sample
  // takes to process.
  for(int s = 0; s < FRAME_OVERLAP; s++){
    timer_start();
    frame_out[s] = filter_sample(&history_in[FRAME_OVERLAP-s-1], 
                                  history_in_exp, 
                                  history_in_hr);
    timer_stop();
  }
}

/**
 * This is the thread entry point for the hardware thread which will actually 
 * be applying the FIR filter.
 * 
 * `c_pcm_in` is the channel from which PCM input samples are received.
 * 
 * `c_pcm_out` is the channel to which PCM output samples are sent.
 */
void filter_thread(
    chanend_t c_pcm_in, 
    chanend_t c_pcm_out)
{
  // Buffer used for storing input sample history.
  int32_t frame_history[FRAME_SIZE] = {0};
  // Exponent associated with frame_history[]
  exponent_t frame_history_exp;
  // Headroom associated with frame_history[]
  headroom_t frame_history_hr;

  // Buffer used to hold output samples.
  q1_31 frame_output[FRAME_OVERLAP] = {0};

  // Loop forever
  while(1) {
    // Receive FRAME_OVERLAP new input samples at the beginning of each frame.
    for(int k = 0; k < FRAME_OVERLAP; k++){
      // Read PCM sample from channel
      const int32_t sample_in = (int32_t) chan_in_word(c_pcm_in);
      // Place at beginning of history buffer in reverse order (to match the
      // order of filter coefficients).
      frame_history[FRAME_OVERLAP-k-1] = sample_in;
    }

    // For now, the exponent associated with each new input frame is -31.
    frame_history_exp = -31;

    // Compute headroom of input frame
    frame_history_hr = vect_s32_headroom(&frame_history[0], FRAME_SIZE);

    // Apply the filter to the new frame of audio, producing FRAME_OVERLAP 
    // output samples in frame_output[].
    filter_frame(frame_output, frame_history, 
                 frame_history_exp, frame_history_hr);

    // Send FRAME_OVERLAP new output samples at the end of each frame.
    for(int k = 0; k < FRAME_OVERLAP; k++){
      chan_out_word(c_pcm_out, frame_output[k]);
    }

    // Finally, shift the frame_history[] buffer up FRAME_OVERLAP samples.
    // This is required to maintain ordering of the sample history.
    memmove(&frame_history[FRAME_OVERLAP], &frame_history[0], 
            TAP_COUNT * sizeof(int32_t));
  }
}

