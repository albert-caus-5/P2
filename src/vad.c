#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "vad.h"
#include "pav_analysis.h"

const float FRAME_TIME = 10.0F; /* in ms. */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

typedef struct {
  float zcr;
  float p;
  float am;
} Features;


Features compute_features(const float *x, int N) {
  Features feat;
  feat.p = compute_power (x,N);
  feat.zcr = compute_zcr(x,N,16000);
  feat.am = compute_am(x,N);
  return feat;
}

VAD_DATA * vad_open(float rate, float alpha0, float alpha1) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->count = 0;                                          
  vad_data->N = 0;                                     
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;
  vad_data->alpha0 = alpha0;                                  
  vad_data->alpha1 = alpha1;                                                                  
  vad_data->k0 = 0;                                          
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {

  VAD_STATE state;
  if(vad_data->state == ST_SILENCE || vad_data->state == ST_VOICE){
    state = vad_data->state;
  } else {
    state = ST_SILENCE;
  }

  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

VAD_STATE vad(VAD_DATA *vad_data, float *x) {

  Features f = compute_features(x, vad_data->frame_length);
  vad_data->last_feature = f.p; 

  switch (vad_data->state) {
    case ST_INIT:
      vad_data->N++;
      vad_data->k0=10*log10(vad_data->k0+pow(10,(f.p/10))/vad_data->N);
      vad_data->state = ST_SILENCE;
    break;

    case ST_UNDEF:
    break;

    case ST_SILENCE:
      if (f.p > vad_data->alpha0 || f.zcr > vad_data->alpha0 + vad_data->alpha1){
        vad_data->state = ST_MAYBE_VOICE;
      }
    break;

    case ST_VOICE:
      if (f.p < vad_data->alpha0){
        vad_data->state = ST_MAYBE_SILENCE;
      }
    break;
  
    case ST_MAYBE_VOICE:
      if(vad_data->count >= 71){
        vad_data->count = 0;
        vad_data->state = ST_SILENCE;

      } else if ((f.p > vad_data->k0+vad_data->alpha0+vad_data->alpha1 && (vad_data->count >= 0))){ 
        vad_data->count = 0;
        vad_data->state = ST_VOICE;
        
      } else {
        vad_data->count++;
      }
    break;

    case ST_MAYBE_SILENCE:
      if(((f.p < vad_data->alpha0) && vad_data->count >= 8)){
        vad_data->count = 0;
        vad_data->state = ST_SILENCE;

      } else if (f.p > vad_data->k0+vad_data->alpha0+vad_data->alpha1 && (vad_data->count >= 0)){
        vad_data->count = 0;
        vad_data->state = ST_VOICE;
      } else {
        vad_data->count++;
      }
    break;
  }

  if (vad_data->state == ST_SILENCE ||vad_data->state == ST_VOICE){
    return vad_data->state;
  } else if (vad_data->state == ST_INIT){
    return ST_SILENCE;
  } else {
    return ST_UNDEF;
  }
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}
