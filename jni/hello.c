#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <android/log.h>

char selected_model_path[512] = "";

JNIEXPORT void JNICALL
Java_com_cakru_dodge_MainActivity_setModelPath(JNIEnv *env, jobject thiz, jstring path) {
    const char *native_path = (*env)->GetStringUTFChars(env, path, NULL);
    strncpy(selected_model_path, native_path, 511);
    (*env)->ReleaseStringUTFChars(env, path, native_path);
    __android_log_print(ANDROID_LOG_INFO, "CalcruJNI", "Model Path Set: %s", selected_model_path);
}

JNIEXPORT jstring JNICALL
Java_com_cakru_dodge_MainActivity_prosesDiRust(JNIEnv *env, jobject thiz, jstring input) {
    if (strlen(selected_model_path) == 0) {
        return (*env)->NewStringUTF(env, "Error: Pilih model dulu via ikon Gear!");
    }
    
    // Nanti di sini tempat mesin AI (Candle/Llama.cpp) bekerja membaca selected_model_path
    char response[600];
    sprintf(response, "Menggunakan model: %s\n\nAI lokal belum disuntikkan sepenuhnya, tapi path file sudah siap diproses oleh mesin Rust.", selected_model_path);
    
    return (*env)->NewStringUTF(env, response);
}
