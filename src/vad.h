#ifndef _VAD_H
#define _VAD_H
#include <stdio.h>

/* DONE: add the needed states */
typedef enum {
   ST_UNDEF = 0,
   ST_SILENCE,
   ST_VOICE,
   ST_INIT,
   ST_MAYBE_VOICE,
   ST_MAYBE_SILENCE
 } VAD_STATE;
 

/* Return a string label associated to each state */
const char *state2str(VAD_STATE st);

/* DONE: add the variables needed to control the VAD 
   (counts, thresholds, etc.) */
//Estructura que tindrà vaddata (el autòmata)
typedef struct {
   VAD_STATE state;
   float sampling_rate;
   unsigned int frame_length;
   float last_feature;  /* for debuggin purposes */
 
   // Umbrals
   float k0;            // k0
   float k1;            // k1 = k0 + alpha0
   float k2;            // k2 = k1 + alpha1
 
   // INIT
   int tk0_frames;
   int t_init;
   float k0_sum;
 
   // MAYBE_VOICE
   int t_voice;
   int t_silence;
   int tk1_frames;
   int tk2_frames;
 
   // MAYBE_SILENCE
   int silence_count;
   int min_silence_frames;
 } VAD_DATA;
 


/* Call this function before using VAD: 
   It should return allocated and initialized values of vad_data

   sampling_rate: ... the sampling rate */
VAD_DATA *vad_open(float sampling_rate);

/* vad works frame by frame.
   This function returns the frame size so that the program knows how
   many samples have to be provided */
unsigned int vad_frame_size(VAD_DATA *);

/* Main function. For each 'time', compute the new state 
   It returns:
    ST_UNDEF   (0) : undefined; it needs more frames to take decission
    ST_SILENCE (1) : silence
    ST_VOICE   (2) : voice

    x: input frame
       It is assumed the length is frame_length */
VAD_STATE vad(VAD_DATA *vad_data, float *x, float alpha0, float alpha1);


/* Free memory
   Returns the state of the last (undecided) states. */
VAD_STATE vad_close(VAD_DATA *vad_data);

/* Print actual state of vad, for debug purposes */
void vad_show_state(const VAD_DATA *, FILE *);

#endif
