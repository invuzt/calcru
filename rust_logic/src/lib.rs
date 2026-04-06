use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::time::Instant;

fn eval(expr: &str) -> Result<f64, String> {
    let tokens: Vec<char> = expr.chars().filter(|c| !c.is_whitespace()).collect();
    let mut pos = 0;
    parse_expression(&tokens, &mut pos)
}

fn parse_expression(tokens: &[char], pos: &mut usize) -> Result<f64, String> {
    let mut val = parse_term(tokens, pos)?;
    while *pos < tokens.len() {
        match tokens[*pos] {
            '+' => { *pos += 1; val += parse_term(tokens, pos)?; }
            '-' => { *pos += 1; val -= parse_term(tokens, pos)?; }
            _ => break,
        }
    }
    Ok(val)
}

fn parse_term(tokens: &[char], pos: &mut usize) -> Result<f64, String> {
    let mut val = parse_power(tokens, pos)?;
    while *pos < tokens.len() {
        match tokens[*pos] {
            '*' => { *pos += 1; val *= parse_power(tokens, pos)?; }
            '/' => {
                *pos += 1;
                let divisor = parse_power(tokens, pos)?;
                if divisor == 0.0 { return Err("Div by zero".to_string()); }
                val /= divisor;
            }
            _ => break,
        }
    }
    Ok(val)
}

fn parse_power(tokens: &[char], pos: &mut usize) -> Result<f64, String> {
    let mut val = parse_factor(tokens, pos)?;
    while *pos < tokens.len() && tokens[*pos] == '^' {
        *pos += 1;
        if *pos < tokens.len() && tokens[*pos] == '^' {
            *pos += 1;
            let times = parse_factor(tokens, pos)? as u32;
            let base = val;
            if times == 0 { val = 1.0; }
            else {
                for _ in 1..times {
                    val = base.powf(val);
                    if val.is_infinite() { return Err("Tetration Overflow".to_string()); }
                }
            }
        } else {
            let exponent = parse_power(tokens, pos)?;
            val = val.powf(exponent);
        }
    }
    Ok(val)
}

fn parse_factor(tokens: &[char], pos: &mut usize) -> Result<f64, String> {
    if *pos < tokens.len() && tokens[*pos] == '(' {
        *pos += 1;
        let val = parse_expression(tokens, pos)?;
        if *pos < tokens.len() && tokens[*pos] == ')' {
            *pos += 1;
            return Ok(val);
        }
        return Err("Missing ')'".to_string());
    }
    parse_number(tokens, pos)
}

fn parse_number(tokens: &[char], pos: &mut usize) -> Result<f64, String> {
    let start = *pos;
    while *pos < tokens.len() && (tokens[*pos].is_digit(10) || tokens[*pos] == '.') {
        *pos += 1;
    }
    if start == *pos { return Err("Expected number".to_string()); }
    let s: String = tokens[start..*pos].iter().collect();
    s.parse::<f64>().map_err(|_| "Invalid number".to_string())
}

#[no_mangle]
pub extern "C" fn rust_engine(input: *const c_char) -> *mut c_char {
    let start_time = Instant::now();
    if input.is_null() { return CString::new("[ERROR] Input NULL").unwrap().into_raw(); }
    let c_str = unsafe { CStr::from_ptr(input) };
    let input_str = c_str.to_str().unwrap_or("");
    let calc_result = if input_str.is_empty() { "Format: 2^3 atau 2^^3".to_string() } else {
        match eval(input_str) {
            Ok(res) => format!("Result: {}", res),
            Err(e) => format!("Error: {}", e),
        }
    };
    let final_output = format!("=== SMART CALC V2.1 ===\nInput: {}\n{}\nRuntime: {:?}\n==================", input_str, calc_result, start_time.elapsed());
    CString::new(final_output).unwrap().into_raw()
}

#[no_mangle]
pub extern "C" fn rust_free_string(s: *mut c_char) {
    unsafe { if !s.is_null() { let _ = CString::from_raw(s); } }
}
