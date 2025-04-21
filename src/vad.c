#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "pav_analysis.h"
#include "vad.h"



//Aquí es defineixen 

const float FRAME_TIME = 10.0F; /* in ms. */

/* 
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT"
};

const char *state2str(VAD_STATE st) {
  switch (st) {
      case ST_SILENCE: return "S";
      case ST_VOICE: return "V";
      default: return "UNDEF";
  }
}

float compute_k0_db(const float *x, int frame_len, int tk0_frames) {
  float sum_lin = 0.0;

  for (int i = 0; i < tk0_frames; i++) {
      const float *frame_ptr = x + i * frame_len;
      float power_db = compute_power(frame_ptr, frame_len); // en dB
      sum_lin += powf(10.0f, power_db / 10.0f); // passem a escala lineal
  }

  float mean_lin = sum_lin / tk0_frames;
  float k0 = 10.0f * log10f(mean_lin); // tornem a dB

  return k0;
}

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
} Features;

/* 
 * TODO: Delete and use your own features!
 */


Features compute_features(const float *x, int N,  int tk0_frames) {
  /*
   * Input: x[i] : i=0 .... N-1 
   * Ouput: computed features
   */
  /* 
   * DELETE and include a call to your own functions
   *
   * For the moment, compute random value between 0 and 1 
   */
  Features feat;
    feat.p = compute_k0_db( x, N, tk0_frames); //Funció de la p1 que calcula la potencia
  return feat;
}

/* 
 * TODO: Init the values of vad_data
 */

 VAD_DATA * vad_open(float rate) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;

  // INIT
  vad_data->tk0_frames = 4; //40ms de mostres
  vad_data->t_init = 0;
  vad_data->k0_sum = 0.0;

  // MAYBE_VOICE
  vad_data->tk1_frames = 1;
  vad_data->tk2_frames = 2;
  vad_data->t_voice = 0;
  vad_data->t_silence = 0;

  // MAYBE_SILENCE
  vad_data->silence_count = 0;
  vad_data->min_silence_frames = 1;

  return vad_data;
}


VAD_STATE vad_close(VAD_DATA *vad_data) {
  /* 
   * TODO: decide what to do with the last undecided frames
   */
  VAD_STATE state = vad_data->state;

  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

/* 
 * Done: Implement the Voice Activity Detection 
 * using a Finite State Automata
 */


//Això és l'autòmata, hem afegit alpha0 pq rebi el valor dels umbrals per terminal i així no cal compilar i executar tot el rato
VAD_STATE vad(VAD_DATA *vad_data, float *x, float alpha0, float alpha1) {
  Features f = compute_features(x, vad_data->frame_length, vad_data->tk0_frames);
  vad_data->last_feature = f.p;

  switch (vad_data->state) {

  case ST_INIT:
    vad_data->k0_sum += f.p;
    vad_data->t_init++;
    if (vad_data->t_init >= vad_data->tk0_frames) {
      vad_data->k0 = vad_data->k0_sum / vad_data->tk0_frames;
      vad_data->k1 = vad_data->k0 + alpha0;
      vad_data->k2 = vad_data->k1 + alpha1;

      printf("k0 = %.5f, k1 = %.5f, k2 = %.5f\n", vad_data->k0, vad_data->k1, vad_data->k2);
      vad_data->t_voice = 0;
      vad_data->state = ST_SILENCE;
    }

    break;

  case ST_SILENCE:
    //printf("Estat silenci\n");
    if (f.p > vad_data->k1) {
      //printf("Primer if del silence\n");
      vad_data->t_voice++;
      if (vad_data->t_voice >= vad_data->tk1_frames) {
        //printf("Segon if del silenci\n");
        vad_data->t_voice = 0;
        vad_data->t_silence = 0;
        vad_data->state = ST_MAYBE_VOICE;
      }
    } else {
      //printf("Else del silence\n");
      vad_data->t_voice = 0;
    }
    break;

  case ST_MAYBE_VOICE:
    //printf("Estat maybe voice\n");
    if (f.p > vad_data->k2) {
      vad_data->state = ST_VOICE;
    } else if (f.p > vad_data->k1) {
      vad_data->t_voice++;
      vad_data->t_silence = 0;

      if (vad_data->t_voice >= vad_data->tk2_frames) {
        vad_data->state = ST_SILENCE;
      }
    } else {
      vad_data->t_silence++;
      vad_data->t_voice = 0;

      if (vad_data->t_silence >= vad_data->tk1_frames) {
        vad_data->state = ST_SILENCE;
      }
    }
    break;

  case ST_VOICE:
    //printf("Estat Voice\n");
    if (f.p < vad_data->k1) {
      vad_data->silence_count = 1;
      vad_data->state = ST_MAYBE_SILENCE;
    }
    break;

  case ST_MAYBE_SILENCE:
    //printf("Estat maybe silenci\n");
    if (f.p < vad_data->k1) {
      vad_data->silence_count++;
      if (vad_data->silence_count >= vad_data->min_silence_frames) {
         vad_data->state = ST_SILENCE;
      }
    } else {
      vad_data->state = ST_VOICE;
    }
    break;

  default:
      break;
  }

  // Només retornem estats definits
  if (vad_data->state == ST_SILENCE || vad_data->state == ST_VOICE)
    return vad_data->state;
  else
    return ST_UNDEF;
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%d\t%f\n", vad_data->state, vad_data->last_feature);
}