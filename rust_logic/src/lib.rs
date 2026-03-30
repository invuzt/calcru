use std::os::raw::c_float;

const COUNT: usize = 150; // Naikkan jumlah partikel agar angin terlihat padat

static mut BOXES_X: [f32; COUNT] = [0.0; COUNT];
static mut BOXES_Y: [f32; COUNT] = [0.0; COUNT];
static mut VEL_X: [f32; COUNT] = [0.0; COUNT];
static mut VEL_Y: [f32; COUNT] = [0.0; COUNT];
static mut TOUCH_X: f32 = -1.0;
static mut TOUCH_Y: f32 = -1.0;
static mut INITIALIZED: bool = false;

fn init() {
    unsafe {
        for i in 0..COUNT {
            // Sebarkan partikel secara acak di layar
            BOXES_X[i] = i as f32 / COUNT as f32;
            BOXES_Y[i] = (i as f32 * 7.1).sin().abs();
            // Kecepatan angin dasar (ke kanan)
            VEL_X[i] = 0.005 + (i as f32 * 0.0001);
            VEL_Y[i] = 0.0;
        }
        INITIALIZED = true;
    }
}

#[no_mangle]
pub extern "C" fn set_rust_touch(x: c_float, y: c_float) {
    unsafe { TOUCH_X = x; TOUCH_Y = y; }
}

#[no_mangle]
pub extern "C" fn update_physics() {
    unsafe {
        if !INITIALIZED { init(); }
        static mut TIMER: f32 = 0.0;
        TIMER += 0.01;

        for i in 0..COUNT {
            // 1. Gaya Angin Dasar (Alur Sinusoid)
            let base_wind_y = (BOXES_X[i] * 5.0 + TIMER).sin() * 0.002;
            VEL_Y[i] += base_wind_y;

            // 2. Gaya Menghindar Jari (Repulsion + Vortex)
            if TOUCH_X > 0.0 {
                let dx = BOXES_X[i] - TOUCH_X;
                let dy = BOXES_Y[i] - TOUCH_Y;
                let dist_sq = dx*dx + dy*dy;
                let dist = dist_sq.sqrt();
                
                if dist < 0.25 {
                    // Semakin dekat jari, tolakan semakin kuat
                    let force = 0.001 / (dist + 0.01);
                    
                    // Komponen Tolakan (Menjauh)
                    VEL_X[i] += (dx / dist) * force;
                    VEL_Y[i] += (dy / dist) * force;

                    // Komponen Putaran (Vortex) agar aliran terlihat mulus belok
                    VEL_X[i] -= (dy / dist) * force * 0.5;
                    VEL_Y[i] += (dx / dist) * force * 0.5;
                }
            }
            
            // 3. Update Posisi & Redaman
            BOXES_X[i] += VEL_X[i];
            BOXES_Y[i] += VEL_Y[i];
            
            // Kembalikan VEL_X ke kecepatan dasar
            VEL_X[i] = VEL_X[i] * 0.95 + 0.005 * 0.05;
            VEL_Y[i] *= 0.92; // Redaman Y agar tidak terlalu liar

            // 4. Reset Partikel yang keluar layar (Looping)
            if BOXES_X[i] > 1.0 { 
                BOXES_X[i] = -0.05; // Mulai lagi dari kiri
                BOXES_Y[i] = (i as f32 * 13.3).sin().abs(); // Acak Y baru
            }
            // Batas Y atas bawah
            if BOXES_Y[i] < 0.0 { BOXES_Y[i] = 0.0; VEL_Y[i] *= -0.5; }
            if BOXES_Y[i] > 1.0 { BOXES_Y[i] = 1.0; VEL_Y[i] *= -0.5; }
        }
    }
}

#[no_mangle] pub extern "C" fn get_box_x(i: i32) -> f32 { unsafe { BOXES_X[i as usize] } }
#[no_mangle] pub extern "C" fn get_box_y(i: i32) -> f32 { unsafe { BOXES_Y[i as usize] } }
// Gunakan waktu untuk gradasi warna latar belakang yang halus
#[no_mangle] pub extern "C" fn get_rust_color_r(t: f32) -> f32 { (t * 0.05).sin().abs() * 0.05 }
