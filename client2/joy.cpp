#include "User_gpio.h"

#define THRESHOLD 100
#define PIN 20
#define IN 0
#define OUT 1

int main(int argc, char **argv) {
  int fd = open(DEVICE, O_RDWR);

  if (fd <= 0) return -1;
  if (prepare(fd) == -1) return -1;
  if (GPIOExport(PIN) == -1) return -1;
  if (GPIODirection(PIN, IN) == -1) return -1;

  while (1) {
    
    int x = readadc(fd, 0);
    int y = readadc(fd, 1);
    int state = GPIORead(PIN);

    if (state == -1) return -1;

    const char *Direction = "Center";

    if (x > 512 + THRESHOLD)
        Direction = "Right";
    else if (x < 512 - THRESHOLD)
        Direction = "Left";

    if (y > 512 + THRESHOLD)
        Direction = "Down";
    else if (y < 512 - THRESHOLD)
        Direction = "UP";

    printf("X: %4d, Y: %4d, D: %6s, S: %3s\n",
               x, y, Direction,
               state == 0 ? "ON" : "OFF");

    usleep(1000*100);
  }
}