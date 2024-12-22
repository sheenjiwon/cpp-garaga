#include "User_gpio.h"
#include "client.h"
#include <wiringPi.h> 

#define USER 1 // User PI 1
#define PIN 20 // Button
#define THRESHOLD 100 // for joystick controll
#define IN 0
#define OUT 1

#define PWM0 0 // R Led for HP
#define LED1 22 // Y Led for Boom cooltime
#define LED2 2 // W Led for shield cooltime
#define VIB 17 // Vibrator

#define FULL_PWM 1000000

int sock;
char IP[30] = "192.168.1.7";
int port;
int Button[2];
int Joy[2];

/**************************************************************
 * controller.cpp for User PI 1
 * User to Server: Joystick, Button
 * Server to User: LED, Haptic
***************************************************************/

int main(int argc, char **argv) {

  port = 9000;
  sock = sock_init(IP, port);

  int fd = open(DEVICE, O_RDWR);

  if (fd <= 0) return -1;
  if (prepare(fd) == -1) return -1;
  if (GPIOExport(PIN) == -1) return -1;
  if (GPIODirection(PIN, IN) == -1) return -1;
  if (GPIOExport(LED1) == -1) return -1;
  if (GPIOExport(LED2) == -1) return -1;
  if (GPIOExport(VIB) == -1) return -1;
  if (GPIODirection(LED1, OUT) == -1) return -1;
  if (GPIODirection(LED2, OUT) == -1) return -1;
  if (GPIODirection(VIB, OUT) == -1) return -1;

  PWMExport(PWM0);
  PWMWritePeriod(PWM0, FULL_PWM);
  PWMWriteDutyCycle(PWM0, 0);
  PWMEnable(PWM0);

  int prev_HP = 100;
  int cycle;

  while (1) {
    
    int x = readadc(fd, 0);
    int y = readadc(fd, 1);
    int state = GPIORead(PIN);

    if (state == -1) return -1;
    int direct = -1;

    if (x > 512 + THRESHOLD)
        direct = 1;
    else if (x < 512 - THRESHOLD)
        direct = 0;
    if (y > 512 + THRESHOLD)
        direct = 3;
    else if (y < 512 - THRESHOLD)
        direct = 2;

    Joy[0] = USER - 1;
    Joy[1] = direct;

    Button[0] = 2 + USER - 1;
    Button[1] = state;
  
    
    while (send(sock, Joy, sizeof(Joy), 0) == -1)
    {
        sock = sock_init(IP, port);
        sleep(1);
    }

    while (send(sock, Button, sizeof(Button), 0) == -1)
    {
        sock = sock_init(IP, port);
        sleep(1);
    }

    int HP = indata[0]/5;
    int B_cool = indata[1];
    int S_cool = indata[2];


    if(prev_HP-HP>0) {
        GPIOWrite(VIB, 1);
        cycle = 500;
    }
    if(cycle == 0) GPIOWrite(VIB, 0);
    if(cycle > 0) cycle--;

    PWMWriteDutyCycle(PWM0, FULL_PWM*HP/100);
    GPIOWrite(LED1, !B_cool);
    GPIOWrite(LED2, !S_cool);

    prev_HP = HP;
    
    usleep(100);
  }

  pthread_join(ttid, NULL);
  return 0;
}