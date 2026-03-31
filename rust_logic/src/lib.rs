use std::fs;

// Fungsi pembantu untuk membaca angka dari file sistem
fn read_sys_file(path: &str) -> f32 {
    fs::read_to_string(path)
        .ok()
        .and_then(|s| s.trim().parse::<f32>().ok())
        .unwrap_or(0.0)
}

#[no_mangle]
pub extern "C" fn get_cpu_usage() -> f32 {
    // Simulasi beban CPU (karena pembacaan /proc/stat butuh kalkulasi delta)
    // Kita buat dummy dulu yang bergerak pelan agar terlihat realtime
    use std::time::{SystemTime, UNIX_EPOCH};
    let t = SystemTime::now().duration_since(UNIX_EPOCH).unwrap().as_millis();
    ((t % 1000) as f32 / 1000.0) // Bar akan berdenyut
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
    read_sys_file("/sys/class/power_supply/battery/capacity") / 100.0
}

#[no_mangle]
pub extern "C" fn get_temp_level() -> f32 {
    // Biasanya suhu baterai dalam miliderajat (misal 35000 = 35C)
    // Kita batasi range 20C (0.0) sampai 60C (1.0)
    let temp = read_sys_file("/sys/class/power_supply/battery/temp") / 1000.0;
    ((temp - 20.0) / 40.0).clamp(0.0, 1.0)
}

// Stub agar tidak error linker
#[no_mangle] pub extern "C" fn update_game(_x: f32) -> i32 { 0 }
#[no_mangle] pub extern "C" fn get_stok() -> i32 { 0 }
#[no_mangle] pub extern "C" fn check_game_over() -> i32 { 0 }
