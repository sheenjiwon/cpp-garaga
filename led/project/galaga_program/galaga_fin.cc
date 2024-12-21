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

                case 6:
                    canvas->SetPixel(i, j, 255, 55, 55);
                    break;
                case 7:
                    canvas->SetPixel(i, j, 255, 102, 102);
                    break;
                case 8:
                    canvas->SetPixel(i, j, 255, 0, 0);
                    break;
                case 9:
                    canvas->SetPixel(i, j, 255, 0, 0);
                    break;
                case 10:
                    canvas->SetPixel(i, j, 0, 255, 0);
                    break;
                case 11:
                    canvas->SetPixel(i, j, 55, 255, 55);
                    break;
                case 12:
                    canvas->SetPixel(i, j, 102, 255, 102);
                    break;
                case 13:
                    canvas->SetPixel(i, j, 0, 255, 0);
                    break;
                case 14:
                    canvas->SetPixel(i, j, 0, 255, 0);
                    break;
                case 15:
                    canvas->SetPixel(i, j, 0, 255, 0);
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
