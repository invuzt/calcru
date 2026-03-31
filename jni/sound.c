#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "sound.h"

SLObjectItf engineObj = NULL; SLEngineItf engineItf;
SLObjectItf outputMixObj = NULL; SLObjectItf playerObj = NULL;
SLPlayItf playerPlay; SLBufferQueueItf playerBufferQueue;

void init_sound() {
    slCreateEngine(&engineObj, 0, NULL, 0, NULL, NULL);
    (*engineObj)->Realize(engineObj, SL_BOOLEAN_FALSE);
    (*engineObj)->GetInterface(engineObj, SL_IID_ENGINE, &engineItf);
    (*engineItf)->CreateOutputMix(engineItf, &outputMixObj, 0, NULL, NULL);
    (*outputMixObj)->Realize(outputMixObj, SL_BOOLEAN_FALSE);
}

void play_crash_sound() {
    // Simulasi bunyi: Di level ini kita kirim buffer audio pendek
    // Untuk mempermudah build sekarang, kita fokus ke trigger log dulu
    // agar tidak crash karena konfigurasi buffer yang spesifik tiap HP.
}
