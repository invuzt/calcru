#include <jni.h>
#include <stdlib.h> // Tambahkan untuk free()

// Deklarasi fungsi dari Rust
extern char* get_hello_rust();

JNIEXPORT jstring JNICALL
Java_com_cakru_dodge_MainActivity_stringFromRust(JNIEnv* env, jobject thiz) {

    // 1. Ambil pointer string dari Rust
    char* str = get_hello_rust();

    // 2. Ubah ke jstring untuk Java
    jstring result = (*env)->NewStringUTF(env, str);

    // 3. PENTING: Jika Rust menggunakan into_raw, kita harus bebaskan 
    // agar tidak memory leak. (Idealnya pakai fungsi khusus dari Rust)
    // free(str); 

    return result;
}

