#include <math.h>
#include "pav_analysis.h"
#include <stdio.h>


float compute_power(const float *x, unsigned int N) {
    float sum = 0, res;
    for(int i = 0; i<N; i++){
        sum += (x[i])*(x[i]);
    }
    res = 10*log10(sum/N);
    return res;
}

float compute_am(const float *x, unsigned int N) {
    
    float sum = 0;
    for(int i = 0; i<N; i++){
        sum += x[i];
    }
    return sum/N;
}

float compute_zcr(const float *x, unsigned int N, float fm) {
    float sum = 0;
    int amp_cons = fm/(2*(N-1));
    for(int i = 0; i<N-1; i++){
        if(x[i]*x[i-1]<0){
            sum++;
            }
    }
    return amp_cons*sum;;
    //Comentari per comprovar que el makefile torna a compilar bé
}

float compute_power_window(const float *x, unsigned int N, const float *ventana){
    
    float sum = 0;
    float den = 0;
    for(int i = 0; i<N; i++){
        sum = sum + ((x[i]*ventana[i])*(x[i]*ventana[i]));
        den = den + (ventana[i] * ventana[i]);
    }   
    return 10*log10(sum/den);   
}