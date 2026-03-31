#include <jni.h>

// Beritahu C bahwa fungsi ini ada di file Rust (lib.rs)
extern char* get_hello_rust();

// Nama fungsi ini HARUS SAMA PERSIS dengan struktur package Java
JNIEXPORT jstring JNICALL
Java_com_cakru_dodge_MainActivity_stringFromRust(JNIEnv* env, jobject thiz) {
    
    // Ambil string dari Rust
    char* pesan_dari_rust = get_hello_rust();
    
    // Ubah format string C/Rust menjadi jstring (format Java)
    return (*env)->NewStringUTF(env, pesan_dari_rust);
}
