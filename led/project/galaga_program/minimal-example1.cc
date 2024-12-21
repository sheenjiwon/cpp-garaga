#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>

#include <iostream>
#include <future>
#include "sock_server.h"
#include "sock_client.h"
#include "button_read.h"

using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
  interrupt_received = true;
}

static void DrawOnCanvas(Canvas *canvas) {
  // int center_x = canvas->width() / 2;
  // int center_y = canvas->height() / 2;
  // float radius_max = canvas->width() / 2;
  // float angle_step = 1.0 / 360;

  // for (float a = 0, r = 0; r < radius_max; a += angle_step, r += angle_step) {
  //   if (interrupt_received)
  //     return;
  //   float dot_x = cos(a * 2 * M_PI) * r;
  //   float dot_y = sin(a * 2 * M_PI) * r;
  //   canvas->SetPixel(center_x + dot_x, center_y + dot_y, 122, 123, 64);
  //   usleep(1 * 1); // wait a little to slow down things.
  // }

  for(int i=0; i<32; i++){
    for(int j=0; j<64; j++){
      canvas->SetPixel(j,i, 122, 122, 64);
      usleep(1 * 1000);
    }
  }
  for(int i=0; i<32; i++){
    for(int j=0; j<64; j++){
      canvas->SetPixel(j,i, 0, 0, 0);
      usleep(1 * 1000);
    }
  }
}

int main(int argc, char *argv[]) {

  // serve server(9001);

  RGBMatrix::Options defaults;
  defaults.hardware_mapping = "regular";
  defaults.rows = 32;
  defaults.chain_length = 1;
  defaults.parallel = 1;
  defaults.show_refresh_rate = true;
  Canvas *canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);
  if (canvas == NULL)
    return 1;

  signal(SIGTERM, InterruptHandler);
  signal(SIGINT, InterruptHandler);

  // while (true) {
  //     int num = server.sock_serv().front() - '0';
  //     std::cout << "wa: " << num << std::endl;
  //     if (num == 1){
  //       DrawOnCanvas(canvas);
  //       canvas->Clear();
  //     }
  // }
  
  while(true){
    DrawOnCanvas(canvas);
    canvas->Clear();
  }
  
  delete canvas;
  return 0;
}
