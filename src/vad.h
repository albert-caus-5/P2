#ifndef _VAD_H
#define _VAD_H
#include <stdio.h>

typedef enum {ST_UNDEF=0, ST_SILENCE, ST_VOICE, ST_INIT, ST_MAYBE_SILENCE, ST_MAYBE_VOICE} VAD_STATE; 

const char *state2str(VAD_STATE st);


typedef struct {
  VAD_STATE state;
  float sampling_rate;
  unsigned int frame_length;
  float last_feature; 
  float k0;
  float N;
  float count;

  float alpha0;
  float alpha1;

} VAD_DATA;


VAD_DATA *vad_open(float sampling_rate, float alpha0, float alpha1);

unsigned int vad_frame_size(VAD_DATA *);


VAD_STATE vad(VAD_DATA *vad_data, float *x);

VAD_STATE vad_close(VAD_DATA *vad_data);

void vad_show_state(const VAD_DATA *, FILE *);

#endif