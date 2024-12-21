#include "led-matrix.h"
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

#include <chrono>
#include <thread>
#include <cmath>
#include <future>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

static struct termios initial_settings, new_settings;

static int peek_character = -1;

void init_keyboard()
{
    tcgetattr(0, &initial_settings);
    new_settings = initial_settings;
    new_settings.c_lflag &= ~ICANON;
    new_settings.c_lflag &= ~ECHO;
    new_settings.c_cc[VMIN] = 1;
    new_settings.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
}

void close_keyboard()
{
    tcsetattr(0, TCSANOW, &initial_settings);
}

int _kbhit()
{
    unsigned char ch;
    int nread;

    if (peek_character != -1)
        return 1;
    new_settings.c_cc[VMIN] = 0;
    tcsetattr(0, TCSANOW, &new_settings);
    nread = read(0, &ch, 1);
    new_settings.c_cc[VMIN] = 1;
    tcsetattr(0, TCSANOW, &new_settings);
    if (nread == 1)
    {
        peek_character = ch;
        return 1;
    }
    return 0;
}


int _getch()
{
    char ch;

    if (peek_character != -1)
    {
        ch = peek_character;
        peek_character = -1;
        return ch;
    }
    read(0, &ch, 1);
    return ch;
}

// void init_keyboard()
// {
//     tcgetattr(0, &initial_settings);      // 현재 터미널 설정 저장
//     new_settings = initial_settings;     // 새로운 설정 복사
//     new_settings.c_lflag &= ~ICANON;     // Canonical 모드 비활성화
//     new_settings.c_lflag &= ~ECHO;       // 입력 문자 출력 비활성화
//     new_settings.c_cc[VMIN] = 1;         // 최소 입력 크기 1바이트
//     new_settings.c_cc[VTIME] = 0;        // 타임아웃 없음
//     tcsetattr(0, TCSANOW, &new_settings); // 새로운 설정 적용
// }

// void close_keyboard()
// {
//     tcsetattr(0, TCSANOW, &initial_settings); // 초기 설정 복원
// }

// // 키 입력 확인 함수 (_kbhit)
// int _kbhit()
// {
//     struct timeval tv = {0L, 0L}; // 타임아웃 없는 timeval 구조체
//     fd_set fds;
//     FD_ZERO(&fds);               // 파일 디스크립터 셋 초기화
//     FD_SET(0, &fds);             // 표준 입력 (파일 디스크립터 0) 추가

//     return select(1, &fds, NULL, NULL, &tv); // 입력이 있으면 1 반환
// }

// // 키 입력 읽기 함수 (_getch)
// int _getch()
// {
//     char ch;
//     read(0, &ch, 1); // 1바이트 읽기
//     return ch;
// }

#define SIZER 64
#define SIZEC 32

using namespace std;

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> dis(0, 255);

const int TARGET_FPS = 100;
const int FRAME_DURATION = 1000 / TARGET_FPS;

#define GENTIME 222
#define CLEARTIME 1000
#define SPEED 20
#define COOLTIME 3000
#define LIFETIME 100
#define SHOOTDELAY 1
#define BOOMCOOLTIME 3000

#define INF 1e9

char mp[SIZER + 1][SIZEC + 1];
char back[SIZER + 1][SIZEC + 1];

using rgb_matrix::Canvas;
using rgb_matrix::RGBMatrix;

RGBMatrix::Options defaults;
Canvas *canvas;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo)
{
    interrupt_received = true;
}

// 색 처리 수정 필요

class Map
{
public:
    void print()
    {
        for (int i = 0; i < SIZER; i++)
        {
            for (int j = 0; j < SIZEC; j++)
            {
                if (mp[i][j] != back[i][j])
                {
                    switch (back[i][j]){
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
                    mp[i][j] = back[i][j];
                }
            }
        }
    }

    void flush()
    {
        memset(back, 0, sizeof back);
    }
};

class Effect
{
    const int Duration = 7000;
    const int Slow = 1000;
    pair<int, int> shape[50];
    int shapeCnt = 0;

public:
    int x, y;
    int liveTime;
    int color;
    bool live = 0;
    Effect(int x, int y, int c) : x(x), y(y), color(c) {}
    Effect() {}
    void init(int a, int b, int c)
    {
        x = a, y = b;
        color = c;
        liveTime = 0;
        live = 1;
        shapeCnt = 0;
    }

    void draw()
    {

        if (liveTime % Slow)
        {
            for (int i = 0; i < shapeCnt; i++)
            {
                auto &p = shape[i];
                back[p.second][p.first] = '!';
            }
            return;
        }

        int ny, tmp;
        shapeCnt = 0;
        for (int i = 1; i < SIZEC - 1; i++)
        {
            if (liveTime / Slow > abs(x - i))
            {
                if (liveTime / Slow * liveTime / Slow - (x - i) * (x - i) < 0)
                    continue;
                tmp = (int)sqrt(liveTime / Slow * liveTime / Slow - (x - i) * (x - i));
                ny = -tmp + y;
                if (ny > 0 && ny < SIZER - 2)
                {
                    shape[shapeCnt++] = {i, ny};
                }
                ny = tmp + y;
                if (ny > 0 && ny < SIZER - 2)
                {
                    shape[shapeCnt++] = {i, ny};
                }
            }
        }
    }
    bool cycle()
    {
        if (!live)
            return 0;
        draw();
        liveTime++;
        if (liveTime >= Duration)
        {
            shapeCnt = 0;
            live = 0;
        }
        return 1;
    }
} effect;

class Bullet
{
public:
    const int movingCycle = 200;

    int x, y, dx, dy;
    int damage;
    int cycle = 0;
    int tag;
    bool live;
    Bullet(int x, int y, int dx = 0, int dy = 0, int damage = 0, int tg = 0) : x(x), y(y), dx(dx), dy(dy), live(1), damage(damage), tag(tg) {}
    Bullet() {}

    void init(int a, int b, int da, int db, int dam, int tg)
    {
        x = a;
        y = b;
        dx = da;
        dy = db;
        damage = dam;
        tag = tg;
        live = 1;
    }
    bool move()
    {
        cycle++;
        if (cycle < movingCycle)
            return 1;
        cycle = 0;
        x += dx, y += dy;

        if (x < 1 || x > SIZEC - 2)
            dx = -dx;

        return y < SIZER - 2 && y > 0;
    }

    void draw()
    {
        back[y][x] = '$';
    }
};

class Type1 : public Bullet
{
public:
    Type1(int x, int y, int dx = 0, int dy = 0, int damage = 1)
    {
        Bullet(x, y, dx, dy, damage);
    }
    bool move()
    {
        cycle++;
        if (cycle < movingCycle)
            return 1;
        cycle = 0;
        x += dx, y += dy;

        if (x < 1 || x > SIZEC - 2)
            dx = -dx;

        return y < SIZER - 2 && y > 0;
    }
};

class ObjectPool
{
public:
    vector<Bullet> pool;
    vector<Effect> efectPool;

    void get(int x, int y, int dx, int dy, int dam, int tag)
    {
        for (auto &p : pool)
        {
            if (!p.live)
            {
                p.init(x, y, dx, dy, dam, tag);
                return;
            }
        }
        pool.push_back(Bullet(x, y, dx, dy, dam, tag));
    }

    void get_effect(int x, int y, int col)
    {
        for (auto &p : efectPool)
        {
            if (!p.live)
            {
                p.init(x, y, col);
                return;
            }
        }
        efectPool.push_back(Effect(x, y, col));
    }

} objectpool;

class Boom
{
    const int Duration = 20000;
    const int Slow = 100;
    pair<int, int> shape[50];
    int shapeCnt = 0;

public:
    const int Cooltime = 10000;

    int x, y;
    int liveTime;
    int tag;
    bool live = 0;
    const int damage = 100;
    Boom(int x, int y, int tag) : x(x), y(y), tag(tag) {}
    Boom() {}
    void move(int a, int b) {}

    void draw()
    {
        if (liveTime % Slow)
        {
            for (int i = 0; i < shapeCnt; i++)
            {
                auto &p = shape[i];
                back[p.second][p.first] = '@';
            }
            return;
        }

        int ny, tmp;
        shapeCnt = 0;
        for (int i = 1; i < SIZEC - 1; i++)
        {
            if (liveTime / Slow > abs(x - i))
            {
                tmp = (int)sqrt(liveTime / Slow * liveTime / Slow - (x - i) * (x - i));
                ny = -tmp + y;
                if (ny > 0 && ny < SIZER - 2)
                {
                    shape[shapeCnt++] = {i, ny};
                }
                ny = tmp + y;
                if (ny > 0 && ny < SIZER - 2)
                {
                    shape[shapeCnt++] = {i, ny};
                }
            }
        }
    }

    void use(int a, int b)
    {
        x = a, y = b;
        liveTime = 0;
        live = 1;
        shapeCnt = 0;
    }

    bool cycle()
    {
        if (!live)
            return 0;

        draw();
        liveTime++;
        if (liveTime >= Duration)
            live = 0;
        return 1;
    }

    bool collision(int a, int b)
    {
        if (!liveTime)
            return 0;

        if (liveTime % Slow)
        {
            for (int i = 0; i < shapeCnt; i++)
            {
                auto &p = shape[i];
                if (p.first == a && p.second == b)
                    return 1;
            }
        }

        return 0;
    }
};

class Shield
{
    const int Duration = 9000;
    vector<pair<int, int>> shape = {{-3, 0}, {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {0, -2}, {1, -2}, {1, 2}, {2, -1}, {3, 0}, {2, 1}, {2, 2}, {-4, 1}, {-4, 2}, {4, 1}, {4, 2}};

public:
    const int Cooltime = 100000;

    int rev;
    int x, y;
    int liveTime;
    int tag;
    bool live = 0;
    Shield(int x, int y, int tag) : x(x), y(y), tag(tag)
    {
        rev = (tag == 1 ? 1 : -1);
    }
    Shield() {}

    void move(int a, int b)
    {
        x = a, y = b;
    }

    void draw()
    {
        for (auto &p : shape)
        {
            back[(y + p.second * rev + SIZER) % SIZER][(x + p.first + SIZEC) % SIZEC] = '~';
        }
    }

    void use(int a, int b)
    {
        x = a, y = b;
        liveTime = 0;
        live = 1;
    }

    bool cycle(int a, int b)
    {
        if (!live)
            return 0;
        move(a, b);
        draw();
        liveTime++;
        if (liveTime >= Duration)
            live = 0;
        return 1;
    }

    bool collision(int a, int b)
    {
        if (!live)
            return 0;
        for (auto &p : shape)
        {
            if (a == x + p.first && b == y + p.second)
                return 1;
        }
        return 0;
    }
};

class Player
{
    const double speed = 1;
    const int shootCooltime = 4000;
    const int ghostTime = 1000;
    int shootMod = 0;

public:
    int ghost = 0;

    double float_x, float_y;
    int x, y;
    int hp;
    int tag;
    int rev;

    Shield *shield;
    Boom *boom;
    int shieldCooldown;
    int boomCooldown;
    int shootCooldown;

    vector<pair<int, int>> shape = {{0, -1}, {0, 0}, {-1, 0}, {1, 0}, {-1, 1}, {1, 1}, {-2, 1}, {-2, 2}, {-2, 3}, {2, 1}, {2, 2}, {2, 3}, {-1, 2}, {1, 2}};
    // 12개

    Player(int x, int y, int hp, int t) : x(x), y(y), hp(hp), tag(t), float_x(x), float_y(y)
    {
        shield = new Shield(x, y, t);
        boom = new Boom(x, y, t);
        shieldCooldown = 0;
        boomCooldown = 0;
        rev = (t == 1 ? 1 : -1);
    }

    void draw()
    {
        for (auto &p : shape)
        {
            back[(y + p.second * rev + SIZER) % SIZER][(x + p.first + SIZEC) % SIZEC] = '*';
        }
    }

    void move_left()
    {
        float_x -= speed;
    }
    void move_right()
    {
        float_x += speed;
    }

    bool use_shield()
    {
        if (shieldCooldown > 0)
        {
            return 0;
        }
        shieldCooldown = shield->Cooltime;
        shield->use(x, y);
        return 1;
    }
    bool use_boom()
    {
        if (boomCooldown > 0)
        {
            return 0;
        }
        boomCooldown = boom->Cooltime;
        boom->use(x, y);
        return 1;
    }

    bool collision(int a, int b)
    {
        for (auto p : shape)
        {
            if ((x + p.first + SIZEC) % SIZEC == a && (y + p.second + SIZER) % SIZER == b)
                return 1;
        }
        return 0;
    }

    void shoot1()
    {
        if (shootCooldown > 0)
        {
            return;
        }
        shootCooldown = shootCooltime;
        objectpool.get(x, y + 2 * -rev, 0, -rev, 5, tag);
    }
    void shoot2()
    {
        if (shootCooldown > 0)
        {
            return;
        }
        shootCooldown = shootCooltime;
        objectpool.get(x, y + 2 * -rev, -1, -rev, 1, tag);
        objectpool.get(x, y + 2 * -rev, 1, -rev, 1, tag);
    }
    void shoot3()
    {
        if (shootCooldown > 0)
        {
            return;
        }
        shootCooldown = shootCooltime * 10;
        objectpool.get(x - 1, y + 2 * -rev, 0, -rev * 3, 10, tag);
        objectpool.get(x + 1, y + 2 * -rev, 0, -rev * 3, 10, tag);
        objectpool.get(x, y + 2 * -rev, 0, -rev * 3, 10, tag);
        objectpool.get(x, y + 1 * -rev, 0, -rev * 3, 10, tag);
        objectpool.get(x, y + 3 * -rev, 0, -rev * 3, 10, tag);

        objectpool.get(x + 2, y + 3 * -rev, 0, -rev * 3, 10, tag);
        objectpool.get(x - 2, y + 3 * -rev, 0, -rev * 3, 10, tag);
    }

    void shoot()
    {
        if (shootMod % 3 == 0)
        {
            shoot1();
        }
        else if (shootMod % 3 == 1)
        {
            shoot2();
        }
        else
        {
            shoot3();
        }
    }

    bool cycle()
    {
        x = ((int)round(float_x) % SIZEC + SIZEC) % SIZEC;
        y = ((int)round(float_y) % SIZER + SIZER) % SIZER;
        shield->cycle(x, y);
        boom->cycle();
        shoot();
        if (shieldCooldown > 0)
            shieldCooldown--;
        if (boomCooldown > 0)
            boomCooldown--;
        if (shootCooldown > 0)
            shootCooldown--;
        if (ghost > 0)
            ghost--;

        return 1;
    }

    void Ghost()
    {
        ghost = ghostTime;
    }

    bool hit(int x)
    {
        if (shield->live || ghost > 0)
            return 0;

        hp -= x;
        return 1;
    }
    void change_mod()
    {
        shootMod++;
        shootCooldown = shootCooltime;
        shootMod = shootMod % 3;
    }
};

class ObjectManager
{
public:
    Player *player1, *player2;
    Player *p;

    void move()
    {
        for (auto &b : objectpool.pool)
        {
            if (!b.live)
                continue;
            if (!b.move())
            {
                b.live = 0;
                continue;
            }

            if (player1->tag == b.tag)
                p = player2;
            else
                p = player1;

            if (p->boom->collision(b.x, b.y))
            {
                objectpool.get_effect(b.x, b.y, 1);
                b.live = 0;
                continue;
            }

            if (p->shield->collision(b.x, b.y))
            {
                b.live = 0;
            }
            else if (p->collision(b.x, b.y))
            {
                b.live = 0;
                p->hit(b.damage);
            }
        }
    }

    void draw()
    {
        for (auto &p : objectpool.efectPool)
        {
            if (!p.live)
                continue;
            p.cycle();
        }
        for (auto &b : objectpool.pool)
        {
            if (!b.live)
                continue;
            b.draw();
        }

        player1->draw();
        player2->draw();
    }

    void cycle()
    {
        player1->cycle();
        player2->cycle();

        move();

        if (player1->boom->collision(player2->x, player2->y))
        {
            if (player2->hit(player1->boom->damage))
            {
                objectpool.get_effect(player2->x, player2->y, 3);
                player2->Ghost();
            }
        }

        if (player2->boom->collision(player1->x, player1->y))
        {
            if (player1->hit(player2->boom->damage))
            {
                objectpool.get_effect(player1->x, player1->y, 3);
                player1->Ghost();
            }
        }

        draw();
    }
};

class UI
{
    int py1 = SIZER - 1, py2 = 0;

public:
    void update(Player &p1, Player &p2)
    {
        // back[py1][0] = 'H';
        // back[py1][1] = 'P';
        // back[py1][2] = ':';
        // for (int i = 0; i < p1.hp / 100 + 1; i++)
        // {
        //     back[py1][3 + i] = '#';
        // }
        // for (int i = p1.hp / 100 + 1; i < 11; i++)
        // {
        //     back[py1][3 + i] = 0;
        // }
        // back[py2][0] = 'H';
        // back[py2][1] = 'P';
        // back[py2][2] = ':';
        // for (int i = 0; i < p2.hp / 100 + 1; i++)
        // {
        //     back[py2][3 + i] = '#';
        // }
        // for (int i = p2.hp / 100 + 1; i < 11; i++)
        // {
        //     back[py2][3 + i] = 0;
        // }
    }
};

class GameManager
{
public:
    ObjectManager objectManager;
    UI ui;
    Map map;

    void key_process1()
    {
        int m_key;

        if (_kbhit()) // 키입력을 체크
        {

            m_key = _getch();
            switch (m_key)
            {
            case 'j':
                objectManager.player1->move_left();
                break;
            case 'l':
                objectManager.player1->move_right();
                break;
            case 'i':
                objectManager.player1->change_mod();
                break;
            case 'k':
                objectManager.player1->use_boom();
                break;
            case '\/':
                objectManager.player1->use_shield();
                break;
            }
        }
    }

    void key_process2()
    {
        int m_key;

        if (_kbhit()) // 키입력을 체크
        {

            m_key = _getch();
            switch (m_key)
            {
            case 'a':
                objectManager.player2->move_left();
                break;
            case 'd':
                objectManager.player2->move_right();
                break;
            case 'w':
                objectManager.player2->change_mod();
                break;
            case 's':
                objectManager.player2->use_boom();
                break;
            case 'z':
                objectManager.player2->use_shield();
                break;
            }
        }
    }

    bool update()
    {
        map.flush();
        objectManager.cycle();
        ui.update(*objectManager.player1, *objectManager.player2);

        return objectManager.player1->hp > 0 && objectManager.player2->hp > 0;
    }
    void rander()
    {
        map.print();
    }

    const double MS_PER_UPDATE = 0.03;

    void start()
    {
        objectManager.player1 = new Player(16, 58, 1000, 1);
        objectManager.player2 = new Player(16, 5, 1000, 2);

        while (1)
        {

            key_process1();
            key_process2();

            if (!update())
            {
                break;
            }

            rander();
        }
        finish();
    }

    void finish()
    {
    }
};

GameManager gm;

int main(int argc, char *argv[])
{
    defaults.hardware_mapping = "regular"; // or e.g. "adafruit-hat"
    defaults.rows = 32;
    defaults.chain_length = 1;
    defaults.parallel = 1;
    defaults.show_refresh_rate = true;
    canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);
    if (canvas == NULL)
        return 1;
    init_keyboard();

    gm.start();

    return 0;
}
