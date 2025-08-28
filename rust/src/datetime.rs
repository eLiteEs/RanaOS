extern "C" {
    // Date and time functions
    pub fn get_second() -> u32;
    pub fn get_minute() -> u32;
    pub fn get_hour() -> u32;
    pub fn get_day() -> u32;
    pub fn get_month() -> u32;
    pub fn get_year() -> u32;
}

pub struct DateTime;

impl DateTime {
    pub fn seconds() -> u32 {
        return unsafe { get_second() };
    }

    pub fn minutes() -> u32 {
        return unsafe { get_minute() };
    }

    pub fn hours() -> u32 {
        return unsafe { get_hour() };
    }

    pub fn day() -> u32 {
        return unsafe { get_day() };
    }

    pub fn month() -> u32 {
        return unsafe { get_month() };
    }

    pub fn year() -> u32 {
        return unsafe { get_year() };
    }
    
    pub fn format(&self) -> [u8; 21] {
        let mut buf = [b' '; 21];
    
        let write_two = |buf: &mut [u8], pos: usize, n: u32| {
            buf[pos] = b'0' + ((n / 10) % 10) as u8;
            buf[pos + 1] = b'0' + (n % 10) as u8;
        };
    
        write_two(&mut buf, 0, DateTime::hours() + 2);
        buf[2] = b':';
        write_two(&mut buf, 3, DateTime::minutes());
        buf[5] = b':';
        write_two(&mut buf, 6, DateTime::seconds());
        buf[8] = b' ';
        buf[9] = b'-';
        buf[10] = b' ';
        write_two(&mut buf, 11, DateTime::day());
        buf[13] = b'/';
        write_two(&mut buf, 14, DateTime::month());
        buf[16] = b'/';
    
        let year = DateTime::year(); // full year
        buf[17] = b'2';
        buf[18] = b'0';
        buf[19] = b'0' + ((year / 10) % 10) as u8;
        buf[20] = b'0' + (year % 10) as u8;
    
        buf
    }
}
