use std::fs::{File, OpenOptions};
use std::io::{Read, Write};
use std::thread::sleep;
use std::time::{Duration, Instant};

// Use GPIO pin 60 = P9_12 on the BeagleBone Black
const GPIO_PIN: &str = "60";
const GPIO_PATH: &str = "/sys/class/gpio";

// Sleep helper in microseconds
fn delay_us(us: u64) {
    sleep(Duration::from_micros(us));
}

// Export GPIO pin for use
fn gpio_export(pin: &str) {
    if let Ok(mut f) = OpenOptions::new().write(true).open(format!("{}/export", GPIO_PATH)) {
        let _ = f.write_all(pin.as_bytes());
    }
    sleep(Duration::from_millis(100));
}

// Set GPIO direction (in or out)
fn gpio_set_direction(pin: &str, direction: &str) {
    let path = format!("{}/gpio{}/direction", GPIO_PATH, pin);
    if let Ok(mut f) = OpenOptions::new().write(true).open(path) {
        let _ = f.write_all(direction.as_bytes());
    }
}

// Write value to GPIO
fn gpio_write(pin: &str, value: u8) {
    let path = format!("{}/gpio{}/value", GPIO_PATH, pin);
    if let Ok(mut f) = OpenOptions::new().write(true).open(path) {
        let _ = f.write_all(if value == 0 { b"0" } else { b"1" });
    }
}

// Read value from GPIO
fn gpio_read(pin: &str) -> i32 {
    let path = format!("{}/gpio{}/value", GPIO_PATH, pin);
    if let Ok(mut f) = File::open(path) {
        let mut buf = [0u8];
        if f.read(&mut buf).is_ok() {
            return if buf[0] == b'1' { 1 } else { 0 };
        }
    }
    -1
}

// Wait until GPIO level matches target, with timeout in μs
fn wait_for_level(pin: &str, level: i32, timeout_us: u64) -> Option<u64> {
    let start = Instant::now();
    let mut elapsed;
    loop {
        if gpio_read(pin) == level {
            elapsed = start.elapsed().as_micros() as u64;
            return Some(elapsed);
        }
        if start.elapsed().as_micros() as u64 > timeout_us {
            return None;
        }
    }
}

// Read 5 bytes (40 bits) of sensor data into array
fn read_sensor_data(pin: &str) -> Result<[u8; 5], &'static str> {
    let mut data = [0u8; 5];

    // Send start signal: pull low for >800μs
    gpio_set_direction(pin, "out");
    gpio_write(pin, 0);
    sleep(Duration::from_millis(1));
    gpio_write(pin, 1);
    delay_us(30);

    // Switch to input to listen
    gpio_set_direction(pin, "in");

    // Wait for sensor response: 80μs low + 80μs high
    if wait_for_level(pin, 0, 100).is_none() { return Err("No response (LOW)"); }
    if wait_for_level(pin, 1, 100).is_none() { return Err("No response (HIGH)"); }

    // Read 40 bits: humidity (16b), temp (16b), checksum (8b)
    for i in 0..5 {
        for j in 0..8 {
            if wait_for_level(pin, 0, 60).is_none() { return Err("Timeout before bit"); }

            let high_time = wait_for_level(pin, 1, 80).unwrap_or(0);
            delay_us(30);

            if wait_for_level(pin, 0, 80).is_none() { return Err("Timeout after bit"); }

            data[i] <<= 1;
            if high_time > 40 { data[i] |= 1; }
        }
    }

    // Check checksum
    let checksum = data[0].wrapping_add(data[1])
        .wrapping_add(data[2])
        .wrapping_add(data[3]);
    if checksum != data[4] {
        return Err("Checksum mismatch");
    }

    Ok(data)
}

// Parse and print the temperature and humidity
fn print_readings(data: [u8; 5]) {
    let humidity = ((data[0] as u16) << 8) | (data[1] as u16);
    let temp_raw = ((data[2] as u16) << 8) | (data[3] as u16);
    let humidity_f = humidity as f32 / 10.0;

    let mut temp_f = (temp_raw & 0x7FFF) as f32 / 10.0;
    if (temp_raw & 0x8000) != 0 {
        temp_f = -temp_f;
    }

    println!("Humidity: {:.1} %", humidity_f);
    println!("Temperature: {:.1} °C", temp_f);
}

fn main() {
    gpio_export(GPIO_PIN);

    loop {
        match read_sensor_data(GPIO_PIN) {
            Ok(data) => print_readings(data),
            Err(e) => println!("Error: {}", e),
        }

        sleep(Duration::from_secs(2)); // Delay between reads
    }
}
