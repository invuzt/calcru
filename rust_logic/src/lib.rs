use std::os::raw::c_float;

const COUNT: usize = 100;
static mut BOXES_X: [f32; COUNT] = [0.5; COUNT];
static mut BOXES_Y: [f32; COUNT] = [0.5; COUNT];
static mut VEL_X: [f32; COUNT] = [0.0; COUNT];
static mut VEL_Y: [f32; COUNT] = [0.0; COUNT];
static mut TOUCH_X: f32 = -1.0;
static mut TOUCH_Y: f32 = -1.0;
static mut INITIALIZED: bool = false;

fn init() {
    unsafe {
        for i in 0..COUNT {
            BOXES_X[i] = 0.5;
            BOXES_Y[i] = 0.5;
            VEL_X[i] = (i as f32 * 0.1).sin() * 0.02;
            VEL_Y[i] = (i as f32 * 0.1).cos() * 0.02;
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
        for i in 0..COUNT {
            // Update posisi
            BOXES_X[i] += VEL_X[i];
            BOXES_Y[i] += VEL_Y[i];

            // Pantulan dinding
            if BOXES_X[i] < 0.0 || BOXES_X[i] > 1.0 { VEL_X[i] *= -1.0; }
            if BOXES_Y[i] < 0.0 || BOXES_Y[i] > 1.0 { VEL_Y[i] *= -1.0; }

            // Interaksi dengan jari (Efek magnet/tolak)
            let dx = TOUCH_X - BOXES_X[i];
            let dy = TOUCH_Y - BOXES_Y[i];
            let dist = (dx*dx + dy*dy).sqrt();
            if dist < 0.2 && dist > 0.01 {
                VEL_X[i] -= dx * 0.005;
                VEL_Y[i] -= dy * 0.005;
            }
        }
    }
}

#[no_mangle] pub extern "C" fn get_box_x(i: i32) -> f32 { unsafe { BOXES_X[i as usize] } }
#[no_mangle] pub extern "C" fn get_box_y(i: i32) -> f32 { unsafe { BOXES_Y[i as usize] } }
