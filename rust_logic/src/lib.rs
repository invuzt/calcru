use jni::objects::{JClass, JString};
use jni::sys::jstring;
use jni::JNIEnv;
use std::sync::Mutex;
use once_cell::sync::Lazy;

// Simpan path model secara global di sisi Rust
static MODEL_PATH: Lazy<Mutex<String>> = Lazy::new(|| Mutex::new(String::new()));

#[no_mangle]
pub extern "system" fn Java_com_cakru_dodge_MainActivity_setModelPath(
    mut env: JNIEnv,
    _class: JClass,
    path: JString,
) {
    let input: String = env.get_string(&path).expect("Gagal baca path").into();
    let mut p = MODEL_PATH.lock().unwrap();
    *p = input;
}

#[no_mangle]
pub extern "system" fn Java_com_cakru_dodge_MainActivity_prosesDiRust(
    mut env: JNIEnv,
    _class: JClass,
    _input: JString,
) -> jstring {
    let p = MODEL_PATH.lock().unwrap();
    
    if p.is_empty() {
        return env.new_string("Pilih model dulu!").unwrap().into_raw();
    }

    // DISINI TEMPAT MESIN AI BEKERJA
    // Untuk mengetes apakah file benar-benar bisa dibuka oleh Rust:
    let file_metadata = std::fs::metadata(&*p);
    
    let respons = match file_metadata {
        Ok(meta) => format!("Berhasil! Rust mendeteksi file model sebesar {} bytes. Siap memproses Qwen2!", meta.len()),
        Err(_) => "Rust gagal mengakses file model. Periksa izin storage!".to_string(),
    };

    env.new_string(respons).unwrap().into_raw()
}
