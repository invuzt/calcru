#include <jni.h>
#include <stdio.h>
#include <string.h>
#include <android/log.h>

#define LOG_TAG "CalcruAI_JNI"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Fungsi simulasi AI sederhana
char* simulasi_ai(const char* input) {
    if (strstr(input, "siapa")) {
        return "Saya Calcru AI, asisten matematika lokal kamu.";
    } else if (strstr(input, "halo") || strstr(input, "hai")) {
        return "Halo! Ada yang bisa saya bantu hari ini?";
    } else if (strstr(input, "matematika")) {
        return "Tentu! Berikan soalnya (misal: integral x^2 dx).";
    } else {
        return "Pertanyaan menarik! Sayangnya, mode AI penuh memerlukan model Gemma-2b yang berukuran 2GB. Untuk saat ini, saya hanya bisa merespons sapaan sederhana.";
    }
}

JNIEXPORT jstring JNICALL
Java_com_cakru_dodge_MainActivity_prosesDiRust(JNIEnv *env, jobject thiz, jstring input) {
    // 1. Ambil String Input dari Java
    const char *in_str = (*env)->GetStringUTFChars(env, input, NULL);
    if (in_str == NULL) return NULL;
    __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, "Input: %s", in_str);

    // 2. Jalankan Logika Simulasi AI
    char *respons_ai = simulasi_ai(in_str);
    jstring result = (*env)->NewStringUTF(env, respons_ai);

    // 3. Bersihkan memori
    (*env)->ReleaseStringUTFChars(env, input, in_str);
    
    return result;
}
