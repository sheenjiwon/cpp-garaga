#include "User_gpio.h"
#include "client.h"
#include <wiringPi.h>           
#include <lcd.h>                
#include <stdio.h>              
#include <stdlib.h>
#include <string.h>

/**************************************************************
 * controller.cpp for User PI 2
 * User to Server: Joystick, Button
 * Server to User: LCD w/o I2C display, Haptic
***************************************************************/

#define LCD_RS  29 // Register select pin
#define LCD_E   6  // Enable Pin
#define LCD_D4  27 // Data pin 4
#define LCD_D5  26 // Data pin 5
#define LCD_D6  11 // Data pin 6
#define LCD_D7  31 // Data pin 7        

#define USER 2
#define PIN 20
#define THRESHOLD 100
#define IN 0
#define OUT 1

#define VIB 17 // Vibrator

int sock;
char IP[30] = "192.168.1.7";
int port;
int Button[2];
int Joy[2];

int main(int argc, char **argv) {

  port = 9000;
  sock = sock_init(IP, port);

  int fd = open(DEVICE, O_RDWR);

  if (fd <= 0) return -1;
  if (prepare(fd) == -1) return -1;
  if (GPIOExport(PIN) == -1) return -1;
  if (GPIODirection(PIN, IN) == -1) return -1;
  if (GPIOExport(VIB) == -1) return -1;
  if (GPIODirection(VIB, OUT) == -1) return -1;

  int prev_HP = 100;
  int cycle;
  int lcd;
  char str[100];

  wiringPiSetup();

  if (lcd = lcdInit(2, 16, 4, LCD_RS, LCD_E,LCD_D4, LCD_D5, LCD_D6,LCD_D7, 0, 0, 0, 0)) {
        printf("lcd init failed! \n");
        return -1;
    }

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
    int B_cool = indata[1]/1000;
    int S_cool = indata[2]/1000;

    if(prev_HP-HP>0) {
        GPIOWrite(VIB, 1);
        cycle = 500;
    }
    if(cycle == 0) GPIOWrite(VIB, 0);
    if(cycle > 0) cycle--;

    char p2_data[33] = "";
    sprintf(p2_data, "HP:%3d", HP);
    lcdPosition(lcd, 0, 0);
    lcdPuts(lcd, p2_data);
    
    sprintf(p2_data, "Boom: %d Guard: %d", B_cool, S_cool);
    lcdPosition(lcd, 0, 1);
    lcdPuts(lcd, p2_data);

    usleep(100);
  }

  pthread_join(ttid, NULL);
  return 0;
}