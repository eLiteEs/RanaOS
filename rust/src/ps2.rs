use Graphics;

extern "C" {
    // I/O functions
    pub fn outb(port: u16, value: u8);
    pub fn inb(port: u16) -> u8;

    // IRQ functions
    pub fn enable_irq(irq: u8);

    // Keyboard
    pub fn was_c_pressed() -> bool;
    pub fn was_key_pressed(key: char) -> bool;
    pub fn key_pressed() -> char;

    // GRUB Modules
    pub fn rread_file_from_meta(name: *const u8) -> *const u8;
}

// Public Rust interface
pub struct Keyboard;

// Estructura para el estado del ratón
pub struct Mouse {
    pub x: i32,
    pub y: i32,
    prev_x: i32,
    prev_y: i32,
    pub buttons: u8,
    width: u32,
    height: u32,
    saved_pixel: u32
}

impl Keyboard {

    pub fn was_c_pressed() -> bool {
        unsafe { was_c_pressed() }
    }
    
    pub fn was_key_pressed(key: char) -> bool {
        unsafe { was_key_pressed(key) }
    }

    pub fn key_pressed() -> char {
        unsafe { key_pressed() }
    }
}

pub fn u32_to_str(mut n: u32, buf: &mut [u8]) -> &str {
    if n == 0 {
        buf[0] = b'0';
        return core::str::from_utf8(&buf[..1]).unwrap();
    }

    let mut i = buf.len();
    while n > 0 {
        i -= 1;
        buf[i] = b'0' + (n % 10) as u8;
        n /= 10;
    }

    core::str::from_utf8(&buf[i..]).unwrap()
}

pub fn read_file_from_meta(name: &str) -> &str {
    // Crear un buffer con null terminator
    let mut buf = [0u8; 256];
    let bytes = name.as_bytes();
    let len = bytes.len().min(buf.len() - 1);
    buf[..len].copy_from_slice(&bytes[..len]);
    buf[len] = 0; // Null terminator

    unsafe {
        let c_ptr = rread_file_from_meta(buf.as_ptr());
        if c_ptr.is_null() {
            return "";
        }

        // Buscar longitud hasta null
        let mut end = 0;
        while *c_ptr.add(end) != 0 {
            end += 1;
        }

        core::str::from_utf8_unchecked(core::slice::from_raw_parts(c_ptr, end))
    }
}

// Registros PS/2
const PS2_DATA: u16 = 0x60;
const PS2_CMD: u16 = 0x64;

impl Mouse {
    pub fn new(screen_width: u32, screen_height: u32) -> Self {
        Self {
            x: (screen_width / 2) as i32,
            y: (screen_height / 2) as i32,
            prev_x: (screen_width / 2) as i32,
            prev_y: (screen_height / 2) as i32,
            buttons: 0,
            width: screen_width,
            height: screen_height,
            saved_pixel: 0xffffff
        }
    }

    // Inicialización del ratón
    pub unsafe fn init(&mut self) {
        // Habilitar dispositivo auxiliar (ratón)
        self.wait_cmd();
        outb(PS2_CMD, 0xA8);
        
        // Habilitar interrupciones
        self.wait_cmd();
        outb(PS2_CMD, 0x20);
        self.wait_data();
        let status = inb(PS2_DATA) | 0x02;
        self.wait_cmd();
        outb(PS2_CMD, 0x60);
        self.wait_data();
        outb(PS2_DATA, status);
        
        // Configurar ratón
        self.write(0xF6);  // Usar configuraciones por defecto
        self.read();       // ACK
        self.write(0xF4);  // Habilitar streaming
        self.read();       // ACK
    }

    // Esperar a que el controlador esté listo
    fn wait_cmd(&self) {
        while (unsafe { inb(PS2_CMD) } & 0x02) != 0 {}
    }

    fn wait_data(&self) {
        while (unsafe { inb(PS2_CMD) } & 0x01) == 0 {}
    }

    // Enviar comando al ratón
    fn write(&mut self, value: u8) {
        self.wait_cmd();
        unsafe { outb(PS2_CMD, 0xD4); }
        self.wait_cmd();
        unsafe { outb(PS2_DATA, value); }
    }

    // Leer dato del ratón
    fn read(&mut self) -> u8 {
        self.wait_data();
        unsafe { inb(PS2_DATA) }
    }

    // Manejar interrupción del ratón
    pub fn handle_interrupt(&mut self) {
        static mut CYCLE: u8 = 0;
        static mut PACKET: [u8; 3] = [0; 3];

        unsafe {
            match CYCLE {
                0 => {
                    PACKET[0] = self.read();
                    if PACKET[0] & 0x08 != 0 { CYCLE = 1; }
                },
                1 => {
                    PACKET[1] = self.read();
                    CYCLE = 2;
                },
                2 => {
                    PACKET[2] = self.read();
                    
                    // Actualizar posición
                    self.buttons = PACKET[0] & 0x07;
                    self.x += (PACKET[1] as i8) as i32;
                    self.y -= (PACKET[2] as i8) as i32;
                    
                    // Limitar a los bordes de la pantalla
                    self.x = self.x.max(0).min((self.width - 1) as i32);
                    self.y = self.y.max(0).min((self.height - 1) as i32);
                    
                    CYCLE = 0;
                },
                _ => CYCLE = 0,
            }
        }
    }

    pub fn draw(&mut self) {
        // 1. Restaurar fondo en la posición anterior
        self.restore_background(self.prev_x, self.prev_y);

        // 2. Guardar fondo en la posición nueva
        self.save_background(self.x, self.y);

        // 3. Draw cursor
        Graphics::put_pixel(self.x as u32, self.y as u32, 0x000000);

        // 4. Actualizar posición anterior
        self.prev_x = self.x;
        self.prev_y = self.y;
    }

    pub fn save_background(&mut self, x: i32, y: i32) {
        self.saved_pixel = Graphics::get_pixel(x as u32, y as u32); 
    }

    pub fn restore_background(&self, x: i32, y: i32) {
        Graphics::put_pixel(x as u32, y as u32, self.saved_pixel);
    }
}
