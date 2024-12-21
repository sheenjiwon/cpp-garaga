#include <wiringPi.h>           //WiringPi headers
#include <lcd.h>                //LCD headers from WiringPi
#include <stdio.h>              //Needed for the printf function below
#include <stdlib.h>
#include <string.h>

//Pin numbers below are the WiringPi pin numbers

#define LCD_RS  29               //Register select pin
#define LCD_E   6               //Enable Pin
#define LCD_D4  27               //Data pin 4
#define LCD_D5  26               //Data pin 5
#define LCD_D6  11               //Data pin 6
#define LCD_D7  31               //Data pin 7



int main() {
    int lcd; //Handle for LCD
    char str[100];

    memset(str, 0x00, sizeof(str));

    wiringPiSetup(); //Initialise WiringPi

    //Initialise LCD(int rows, int cols, int bits, int rs, int enable, int d0, int d1, int d2, int d3, int d4, int d5, int d6, int d7)
    if (lcd = lcdInit(2, 16, 4, LCD_RS, LCD_E,LCD_D4, LCD_D5, LCD_D6,LCD_D7, 0, 0, 0, 0)) {
        printf("lcd init failed! \n");
        return -1;
    }
    lcdClear(lcd);
    lcdPosition(lcd, 0, 0); //Position cursor on the first line in the first column
    scanf("%[^\n]s", str);
    printf("%s\n", str);
    lcdPuts(lcd, "HI"); //Print the text on the LCD at the current cursor postion
    getchar(); //Wait for key press
    getchar();
}
