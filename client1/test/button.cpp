#include "User_gpio.h"

#define PIN 20
#define IN 0
#define OUT 1

int main(int argc, char* argv[]) {
  
  if (GPIOExport(PIN) == -1) return 1;
  if (GPIODirection(PIN, IN) == -1) return 2;

  while (1) {
    int state = GPIORead(PIN);
    if (state == -1) return 3;

    printf("%s\n", state == 0 ? "ON" : "OFF");
    usleep(1000*100);
  }

  return 0;
}