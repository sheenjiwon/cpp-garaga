#include "server.h"
#include "led-matrix.h"

#include <random>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <future>

extern char pmp[64][32];

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

            switch (pmp[i][j]){
                case 0: canvas->SetPixel(i, j, 0, 0, 0);
                break;
                case '@':
                        canvas->SetPixel(i, j, dis(gen), dis(gen), dis(gen));
                break;
                case '*':
                        canvas->SetPixel(i, j, 0, 255, 0);
                break;
                case '$':
                        canvas->SetPixel(i, j, 0, 0, 255);
                break;
                case '~':
                        canvas->SetPixel(i, j, 122, 122, 60);
                break;
                    case '!':
                        canvas->SetPixel(i, j, dis(gen), dis(gen), dis(gen));
                break;

                default : break;
            }

        }
    }
}

int main(int argc, char *argv[])
{
    defaults.hardware_mapping = "regular";
    defaults.rows = 32;
    defaults.chain_length = 1;
    defaults.parallel = 1;
    defaults.show_refresh_rate = true;
    canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);
    if (canvas == NULL)
        return 1;

    
    int option = 0;
    pthread_t tid, tid2;
    int thr_id = pthread_create(&tid, NULL, t_function_main, &option);

    
    while(1){
        // for(int i=0; i<64; i++){
        //     for(int j=0; j<32; j++){
        //         std::cout<< int(pmp[i][j]) << '\n';
        //     }
        //     cout << '\n';
        // }
        LED_Print();
    }
    

    pthread_join(tid, NULL);

    return 0;
}
