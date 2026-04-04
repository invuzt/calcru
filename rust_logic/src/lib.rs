use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::time::Instant; // Library standar, tidak perlu crate baru
use log::{info, LevelFilter};
use android_logger::Config;

#[no_mangle]
pub extern "C" fn rust_engine(input: *const c_char) -> *mut c_char {
    // Mulai hitung waktu tepat saat data masuk
    let start_time = Instant::now();

    android_logger::init_once(
        Config::default().with_max_level(LevelFilter::Trace).with_tag("CAKRU_RUST")
    );

    if input.is_null() {
        return CString::new("[LOG: Error] Input NULL").unwrap().into_raw();
    }

    let c_str = unsafe { CStr::from_ptr(input) };
    let input_str = c_str.to_str().unwrap_or("error");
    
    let mut log_ui = format!("[LOG: Menerima '{}']\n", input_str);

    let response = match input_str.to_lowercase().as_str() {
        "cek stok" => {
            log_ui.push_str("[LOG: Memindai database Odfiz...]\n");
            "Rust: Stok saat ini ada 150 unit.".to_string()
        },
        "halo" => {
            log_ui.push_str("[LOG: Menginisialisasi greeting...]\n");
            "Rust: Halo Guru! Siap memproses data.".to_string()
        },
        "jadwal" => {
            log_ui.push_str("[LOG: Sinkronisasi jadwal maintenance...]\n");
            "Rust: Jadwal besok adalah shift pagi.".to_string()
        },
        _ => {
            log_ui.push_str("[LOG: Perintah tidak dikenali]\n");
            format!("Rust menerima: '{}'.", input_str)
        },
    };

    // Hitung durasi proses
    let duration = start_time.elapsed();
    log_ui.push_str(&format!("[LOG: Selesai dalam {:?}]\n---\n", duration));
    
    let final_output = format!("{}{}", log_ui, response);
    
    info!("{}", final_output);
    CString::new(final_output).unwrap().into_raw()
}

#[no_mangle]
pub extern "C" fn rust_free_string(s: *mut c_char) {
    unsafe {
        if s.is_null() { return; }
        let _ = CString::from_raw(s);
    }
}
