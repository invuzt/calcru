use std::ffi::CString;
use std::os::raw::c_char;

#[no_mangle]
pub extern "C" fn get_hello_rust() -> *mut c_char {
    // Teks yang ingin dikirim ke Android
    let s = CString::new("Hello World dari Rust! 🦀\nBerhasil Terhubung!").unwrap();
    
    // Kirim ke C sebagai pointer
    s.into_raw()
}
