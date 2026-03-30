use std::os::raw::c_float;

static mut BOX_X: f32 = 0.5;
static mut BOX_Y: f32 = 0.5;

#[no_mangle]
pub extern "C" fn set_rust_touch(x: c_float, y: c_float) {
    unsafe {
        // Kotak akan mengikuti jari
        BOX_X = x;
        BOX_Y = y;
    }
}

#[no_mangle]
pub extern "C" fn get_box_x() -> c_float { unsafe { BOX_X } }

#[no_mangle]
pub extern "C" fn get_box_y() -> c_float { unsafe { BOX_Y } }

#[no_mangle]
pub extern "C" fn get_rust_color_r(t: f32) -> f32 { (t * 0.5).sin().abs() }
