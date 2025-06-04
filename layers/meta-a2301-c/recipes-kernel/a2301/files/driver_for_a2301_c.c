#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>

#define GPIO_PIN "60"  // Use a suitable GPIO pin from P8 or P9 headers
#define GPIO_PATH "/sys/class/gpio"

// Delay helper: microseconds (us)
void delayMicroseconds(unsigned int us) {
    struct timespec ts;
    ts.tv_sec = us / 1000000;
    ts.tv_nsec = (us % 1000000) * 1000;
    nanosleep(&ts, NULL);
}

// Export GPIO pin to sysfs
void gpio_export(const char *pin) {
    int fd = open(GPIO_PATH "/export", O_WRONLY);
    if (fd >= 0) {
        write(fd, pin, strlen(pin));
        close(fd);
    }
    usleep(100000); // Wait for the sysfs to create the GPIO directory
}

// Set GPIO direction (in or out)
void gpio_direction(const char *pin, const char *direction) {
    char path[64];
    snprintf(path, sizeof(path), GPIO_PATH "/gpio%s/direction", pin);
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        write(fd, direction, strlen(direction));
        close(fd);
    }
}

// Write GPIO value (0 or 1)
void gpio_write(const char *pin, int value) {
    char path[64];
    snprintf(path, sizeof(path), GPIO_PATH "/gpio%s/value", pin);
    int fd = open(path, O_WRONLY);
    if (fd >= 0) {
        write(fd, value ? "1" : "0", 1);
        close(fd);
    }
}

// Read GPIO value (returns 0 or 1)
int gpio_read(const char *pin) {
    char path[64], val;
    snprintf(path, sizeof(path), GPIO_PATH "/gpio%s/value", pin);
    int fd = open(path, O_RDONLY);
    if (fd >= 0) {
        read(fd, &val, 1);
        close(fd);
        return val == '1' ? 1 : 0;
    }
    return -1;
}

// Wait for signal level with timeout (μs). Returns duration or -1 if timeout.
int wait_level(const char *pin, int level, int timeout_us) {
    int count = 0;
    while (gpio_read(pin) != level && count++ < timeout_us) {
        delayMicroseconds(1);
    }
    return count >= timeout_us ? -1 : count;
}

// Read 40 bits of sensor data into a buffer
int read_sensor_data(uint8_t *data) {
    int i, j;

    // Step 1: Send start signal (low for 1ms)
    gpio_direction(GPIO_PIN, "out");
    gpio_write(GPIO_PIN, 0);
    usleep(1000); // at least 800μs required
    gpio_write(GPIO_PIN, 1);
    delayMicroseconds(30); // Wait before switching to input

    // Step 2: Wait for sensor response
    gpio_direction(GPIO_PIN, "in");

    if (wait_level(GPIO_PIN, 0, 100) == -1) return -1; // Wait 80μs low
    if (wait_level(GPIO_PIN, 1, 100) == -1) return -1; // Wait 80μs high

    // Step 3: Read 40 bits (5 bytes)
    for (i = 0; i < 5; i++) {
        data[i] = 0;
        for (j = 0; j < 8; j++) {
            if (wait_level(GPIO_PIN, 0, 60) == -1) return -1; // 50μs low before bit

            int high_time = wait_level(GPIO_PIN, 1, 80);
            if (high_time == -1) return -1;

            delayMicroseconds(30); // Wait for bit to settle

            // Interpret bit: > 40μs means '1', else '0'
            if (wait_level(GPIO_PIN, 0, 80) == -1) return -1;
            data[i] <<= 1;
            if (high_time > 40)
                data[i] |= 1;
        }
    }

    // Step 4: Check checksum
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    return (checksum == data[4]) ? 0 : -1;
}

// Convert raw data to float values
void print_readings(uint8_t *data) {
    int humidity = (data[0] << 8) | data[1];
    int temp = (data[2] << 8) | data[3];

    float h = humidity / 10.0;
    float t = (temp & 0x7FFF) / 10.0;
    if (temp & 0x8000) t = -t;

    printf("Humidity: %.1f %%\n", h);
    printf("Temperature: %.1f °C\n", t);
}

int main() {
    uint8_t data[5];

    gpio_export(GPIO_PIN);

    while (1) {
        if (read_sensor_data(data) == 0) {
            print_readings(data);
        } else {
            printf("Failed to read sensor data.\n");
        }

        sleep(2); // Must wait 2 seconds between readings
    }

    return 0;
}
