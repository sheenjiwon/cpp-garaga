# include "User_gpio.h"

const char *DEVICE = "/dev/spidev0.0";
uint8_t MODE = 0;
uint8_t BITS = 8;
uint32_t CLOCK = 500000;
uint16_t DELAY = 0;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define VALUE_MAX 40
#define DIRECTION_MAX 40

#define VALUE_MAX1 256
#define DIRECTION_MAX1 256
#define BUFFER_MAX 3

/**************************************************************
 * GPIO Export
 * GPIO Unexport
 * GPIO Direction
 * GPIO Read
 * GPIO Write
 * PWM Export
 * PWM Enable
 * PWM Write Period
 * PWM Write Dutycycle
***************************************************************/

int GPIOExport(int pin) {
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;

  fd = open("/sys/class/gpio/export", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open export for writing!\n");
    return (-1);
  }

  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
  return (0);
}

int GPIOUnexport(int pin) {
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;

  fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open unexport for writing!\n");
    return (-1);
  }

  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
  return (0);
}

int GPIODirection(int pin, int dir) {
  static const char s_directions_str[] = "in\0out";

  char path[DIRECTION_MAX];
  int fd;

  snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio direction for writing!\n");
    return (-1);
  }

  if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
    fprintf(stderr, "Failed to set direction!\n");
    return (-1);
  }

  close(fd);
  return (0);
}

int GPIORead(int pin) {
  char path[VALUE_MAX];
  char value_str[3];
  int fd;

  snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_RDONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for reading!\n");
    return (-1);
  }

  if (-1 == read(fd, value_str, 3)) {
    fprintf(stderr, "Failed to read value!\n");
    return (-1);
  }

  close(fd);

  return (atoi(value_str));
}

int GPIOWrite(int pin, int value) {
  static const char s_values_str[] = "01";

  char path[VALUE_MAX];
  int fd;

  snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for writing!\n");
    return (-1);
  }

  if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
    fprintf(stderr, "Failed to write value!\n");
    return (-1);
  }

  close(fd);
  return (0);
}

int prepare(int fd) {
  if (ioctl(fd, SPI_IOC_WR_MODE, &MODE) == -1) {
    perror("Can't set MODE");
    return -1;
  }

  if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &BITS) == -1) {
    perror("Can't set number of BITS");
    return -1;
  }

  if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &CLOCK) == -1) {
    perror("Can't set write CLOCK");
    return -1;
  }

  if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &CLOCK) == -1) {
    perror("Can't set read CLOCK");
    return -1;
  }

  return 0;
}

uint8_t control_bits_differential(uint8_t channel) {
  return (channel + 8) << 4;
}

uint8_t control_bits(uint8_t channel) {
  return 0x8 | control_bits_differential(channel);
}

int readadc(int fd, uint8_t channel) {
  uint8_t tx[3] = {1, control_bits(channel), 0};
  uint8_t rx[3] = {0};

  struct spi_ioc_transfer tr = {
      .tx_buf = (unsigned long)tx,
      .rx_buf = (unsigned long)rx,
      .len = ARRAY_SIZE(tx),
      .speed_hz = CLOCK,
      .delay_usecs = DELAY,
      .bits_per_word = BITS,
  };

  if (ioctl(fd, SPI_IOC_MESSAGE(1), &tr) == 1) {
    perror("IO Error");
    abort();
  }
  
  return ((rx[1] & 3) << 8) | rx[2];
}


int PWMExport(int pwmnum) {
#define BUFFER_MAX 3
  char buffer[BUFFER_MAX];
  int fd, byte;

  fd = open("/sys/class/pwm/pwmchip0/export", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open export for export!\n");
    return (-1);
  }

  byte = snprintf(buffer, BUFFER_MAX, "%d", pwmnum);
  write(fd, buffer, byte);
  close(fd);

  sleep(1);

  return (0);
}

int PWMEnable(int pwmnum) {
  static const char s_enable_str[] = "1";

  char path[DIRECTION_MAX1];
  int fd;

  snprintf(path, DIRECTION_MAX1, "/sys/class/pwm/pwmchip0/pwm%d/enable", pwmnum);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open in enable!\n");
    return -1;
  }

  write(fd, s_enable_str, strlen(s_enable_str));
  close(fd);

  return (0);
}

int PWMWritePeriod(int pwmnum, int value) {
  char s_value_str[VALUE_MAX1];
  char path[VALUE_MAX1];
  int fd, byte;

  snprintf(path, VALUE_MAX1, "/sys/class/pwm/pwmchip0/pwm%d/period", pwmnum);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open in period!\n");
    return (-1);
  }
  byte = snprintf(s_value_str, VALUE_MAX1, "%d", value);

  if (-1 == write(fd, s_value_str, byte)) {
    fprintf(stderr, "Failed to write value in period!\n");
    close(fd);
    return -1;
  }
  close(fd);

  return (0);
}

int PWMWriteDutyCycle(int pwmnum, int value) {
  char s_value_str[VALUE_MAX1];
  char path[VALUE_MAX1];
  int fd, byte;

  snprintf(path, VALUE_MAX1, "/sys/class/pwm/pwmchip0/pwm%d/duty_cycle", pwmnum);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open in duty cycle!\n");
    return (-1);
  }
  byte = snprintf(s_value_str, VALUE_MAX1, "%d", value);

  if (-1 == write(fd, s_value_str, byte)) {
    fprintf(stderr, "Failed to write value in duty cycle!\n");
    close(fd);
    return -1;
  }
  close(fd);

  return (0);
}
