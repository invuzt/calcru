use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::time::Instant;

#[no_mangle]
pub extern "C" fn rust_engine(input: *const c_char) -> *mut c_char {
    let start_time = Instant::now();

    if input.is_null() {
        return CString::new("[LOG: Error] Input NULL").unwrap().into_raw();
    }

    let c_str = unsafe { CStr::from_ptr(input) };
    let input_str = c_str.to_str().unwrap_or("error");
    
    let mut log_ui = format!("[LOG: Perintah: '{}']\n", input_str);

    let response = match input_str.to_lowercase().as_str() {
        "mining" | "simulasi" => {
            log_ui.push_str("[LOG: Memulai simulasi 1.000.000 transaksi...]\n");
            
            let mut total_btc_transfered: f64 = 0.0;
            let target_count = 1_000_000;

            // Simulasi proses 1 juta data transfer
            for i in 0..target_count {
                // Simulasi nominal acak sederhana
                total_btc_transfered += 0.0001;
                
                // Setiap 250rb data, kita catat di log internal Rust (opsional)
                if i == 500_000 {
                    // Hanya penanda internal proses mencapai 50%
                }
            }

            format!(
                "=== BITCOIN SIMULATOR ===\n\
                 Status: Block Mined!\n\
                 Data Diolah: {} Transaksi\n\
                 Volume: {:.2} BTC\n\
                 Kecepatan: Sangat Tinggi (Rust Core)\n\
                 =========================", 
                target_count, total_btc_transfered
            )
        },
        _ => format!("Ketik 'mining' untuk simulasi 1 juta data."),
    };

    let duration = start_time.elapsed();
    log_ui.push_str(&format!("[LOG: Selesai dalam {:?}]\n---\n", duration));
    
    let final_output = format!("{}{}", log_ui, response);
    CString::new(final_output).unwrap().into_raw()
}

#[no_mangle]
pub extern "C" fn rust_free_string(s: *mut c_char) {
    unsafe {
        if s.is_null() { return; }
        let _ = CString::from_raw(s);
    }
}
