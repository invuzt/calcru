use std::fs;
use std::io::{Read, Write};
use std::sync::atomic::{AtomicI32, Ordering};

static STOK: AtomicI32 = AtomicI32::new(0);
const PATH: &str = "/data/user/0/com.cakru.dodge/files/stok.txt";

#[no_mangle]
pub extern "C" fn save_stok() {
    let val = STOK.load(Ordering::SeqCst);
    if let Ok(mut f) = fs::File::create(PATH) {
        let _ = f.write_all(val.to_string().as_bytes());
    }
}

#[no_mangle]
pub extern "C" fn load_stok() {
    if let Ok(mut f) = fs::File::open(PATH) {
        let mut s = String::new();
        if f.read_to_string(&mut s).is_ok() {
            if let Ok(n) = s.trim().parse::<i32>() {
                STOK.store(n, Ordering::SeqCst);
            }
        }
    }
}

#[no_mangle]
pub extern "C" fn get_cpu_usage() -> f32 {
    use std::time::{SystemTime, UNIX_EPOCH};
    let t = SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_millis();
    (t % 1000) as f32 / 1000.0
}

#[no_mangle]
pub extern "C" fn get_ram_usage() -> f32 {
    if let Ok(mem) = fs::read_to_string("/proc/meminfo") {
        let mut total = 1.0;
        let mut avail = 0.0;
        for line in mem.lines() {
            if line.starts_with("MemTotal:") {
                total = line.split_whitespace().nth(1).unwrap_or("1").parse().unwrap_or(1.0);
            }
            if line.starts_with("MemAvailable:") {
                avail = line.split_whitespace().nth(1).unwrap_or("0").parse().unwrap_or(0.0);
            }
        }
        return (total - avail) / total;
    }
    0.5
}

#[no_mangle]
pub extern "C" fn get_bat_level() -> f32 {
    fs::read_to_string("/sys/class/power_supply/battery/capacity")
        .ok()
        .and_then(|s| s.trim().parse::<f32>().ok())
        .map(|v| v / 100.0)
        .unwrap_or(0.5)
}

#[no_mangle]
pub extern "C" fn get_temp_level() -> f32 {
    let temp = fs::read_to_string("/sys/class/power_supply/battery/temp")
        .ok()
        .and_then(|s| s.trim().parse::<f32>().ok())
        .unwrap_or(300.0) / 10.0; // Miliderajat ke Derajat
    ((temp - 20.0) / 40.0).clamp(0.0, 1.0)
}

// Fungsi dummy agar Makefile lama tidak protes
#[no_mangle] pub extern "C" fn update_game(_x: f32) -> i32 { 0 }
#[no_mangle] pub extern "C" fn get_stok() -> i32 { 0 }
#[no_mangle] pub extern "C" fn check_game_over() -> i32 { 0 }
#[no_mangle] pub extern "C" fn tambah_stok() {}
#[no_mangle] pub extern "C" fn kurangi_stok() {}
