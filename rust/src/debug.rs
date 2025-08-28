extern "C" {
    // Debug functions
    pub fn debug_print(m: *const u8);
    pub fn debug_print_dec(n: u32);
    pub fn debug_print_hex(h: u32);
}

pub struct Debug;

impl Debug {
    pub fn print(m: &str) {
        let mut buf = [0u8; 256];
        let bytes = m.as_bytes();
        let len = bytes.len().min(buf.len() - 1);
        buf[..len].copy_from_slice(&bytes[..len]);
        buf[len] = 0; // Null terminator

        unsafe { debug_print(buf.as_ptr());  }
    }

    pub fn print_dec(n: u32) {
        unsafe { debug_print_dec(n); }
    }

    pub fn print_hex(h: u32) {
        unsafe { debug_print_hex(h); }
    }
}
