use std::os::raw::c_float;

const COUNT: usize = 100;
const MAX_TOUCH: usize = 10;

static mut BOXES_X: [f32; COUNT] = [0.5; COUNT];
static mut BOXES_Y: [f32; COUNT] = [0.5; COUNT];
static mut VEL_X: [f32; COUNT] = [0.0; COUNT];
static mut VEL_Y: [f32; COUNT] = [0.0; COUNT];
static mut TOUCH_X: [f32; MAX_TOUCH] = [-1.0; MAX_TOUCH];
static mut TOUCH_Y: [f32; MAX_TOUCH] = [-1.0; MAX_TOUCH];

#[no_mangle]
pub extern "C" fn set_rust_touch(id: i32, x: f32, y: f32) {
    unsafe {
        if id >= 0 && id < MAX_TOUCH as i32 {
            TOUCH_X[id as usize] = x;
            TOUCH_Y[id as usize] = y;
        }
    }
}

#[no_mangle]
pub extern "C" fn update_physics() {
    unsafe {
        for i in 0..COUNT {
            let mut force_x = 0.0;
            let mut force_y = 0.0;

            for t in 0..MAX_TOUCH {
                if TOUCH_X[t] < 0.0 { continue; }
                let dx = TOUCH_X[t] - BOXES_X[i];
                let dy = TOUCH_Y[t] - BOXES_Y[i];
                let dist = (dx*dx + dy*dy).sqrt();
                if dist < 0.3 {
                    force_x += dx * 0.005;
                    force_y += dy * 0.005;
                }
            }
            
            VEL_X[i] = (VEL_X[i] + force_x) * 0.92;
            VEL_Y[i] = (VEL_Y[i] + force_y) * 0.92;
            BOXES_X[i] += VEL_X[i];
            BOXES_Y[i] += VEL_Y[i];
        }
    }
}

#[no_mangle] pub extern "C" fn get_box_x(i: i32) -> f32 { unsafe { BOXES_X[i as usize] } }
#[no_mangle] pub extern "C" fn get_box_y(i: i32) -> f32 { unsafe { BOXES_Y[i as usize] } }
#[no_mangle] pub extern "C" fn get_touch_x(i: i32) -> f32 { unsafe { TOUCH_X[i as usize] } }
#[no_mangle] pub extern "C" fn get_touch_y(i: i32) -> f32 { unsafe { TOUCH_Y[i as usize] } }
#[no_mangle] pub extern "C" fn get_rust_color_r(t: f32) -> f32 { (t * 0.05).sin().abs() * 0.1 }
