#include "server.h"
#include "led-matrix.h"

#include <random>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <future>

#define BUF_SIZE 512

using rgb_matrix::Canvas;
using rgb_matrix::RGBMatrix;
using namespace std;

RGBMatrix::Options defaults;
Canvas *canvas;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo)
{
    interrupt_received = true;
}

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> dis(0, 255);

char mp[64][32];

void set_random_red(int i, int j){
    int k = dis(gen) % 3;
    switch(k){
        case 0:  canvas->SetPixel(i, j, 0x00, 0xFF, 0x00);
        break;
        case 1: canvas->SetPixel(i, j, 0x00, 0xFF, 0x7F);
        break;
        case 2: canvas->SetPixel(i, j, 0x46, 0x82, 0xB4);
        break;
        case 3: canvas->SetPixel(i, j, 0x7B, 0x68, 0xEE);
        break;
        case 4:
            canvas->SetPixel(i, j, 0x94, 0x00, 0xD3);
        break;
    }

}

void set_random_blue(int i, int j){
    int k = dis(gen) % 3;
    switch(k){
        case 0:  canvas->SetPixel(i, j, 0xFF, 0x69, 0xB4); break;
        case 1: canvas->SetPixel(i, j, 0xFF,0xFF,0x00);
        break;
        case 2: canvas->SetPixel(i, j, 0xFF,0x69,0xB4);
        break;
        case 3: canvas->SetPixel(i, j, 0x8B,0x00,0x00);
        break;
        case 4:
        canvas->SetPixel(i, j, 0xBA, 0x55, 0xD3);
        break;
    }
}

void LED_Print(){
    for (int i = 0; i < 64; i++)
    {
        for (int j = 0; j < 32; j++)
        {
            if (pmp[i][j] != mp[i][j])
            {
                switch (pmp[i][j])
                {
                case 0:
                    canvas->SetPixel(i, j, 0, 0, 0);
                    break;

                case 11:
                    canvas->SetPixel(i, j, 0xFF,0x8C, 0x00);
                    break;
                case 12:
                    canvas->SetPixel(i, j, 0xDC,0x14,0x3C);
                    break;
                case 13:
                    canvas->SetPixel(i, j, 255, 0, 0);
                    break;
                case 14:
                    canvas->SetPixel(i, j, 255, 0, 0);
                    break;
                case 15:
                    set_random_red(i, j);
                    break;


                case 21:
                    canvas->SetPixel(i, j, 0x00, 0xFF, 0xFF);
                    break;
                case 22:
                    canvas->SetPixel(i, j, 0x00, 0xBF, 0xFF);
                    break;
                case 23:
                     canvas->SetPixel(i, j, 0, 255, 0);
                    break;
                case 24:
                    canvas->SetPixel(i, j, 0, 255, 0);
                    break;
                case 25:
                   set_random_blue(i, j);
                    break;

                
                case 30:
                    canvas->SetPixel(i, j, dis(gen), dis(gen), dis(gen));
                    break;

                default:
                    break;
                }
            }
            mp[i][j] = pmp[i][j];
        }
    }
}


int main(int argc, char *argv[])
{
    defaults.hardware_mapping = "regular";
    defaults.rows = 32;
    defaults.chain_length = 1;
    defaults.parallel = 1;
    defaults.show_refresh_rate = false;
    canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);
    if (canvas == NULL)
        return 1;

    
    int option = 0;
    pthread_t tid, tid2;
    int thr_id = pthread_create(&tid, NULL, t_function_main, &option);

    
    while (1) {
        LED_Print();
        usleep(1000 * 10);
    }

    pthread_join(tid, NULL);

    return 0;
}
