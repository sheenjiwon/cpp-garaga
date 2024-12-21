
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
#include "client.h"
#include "keyboard.h"
#include "server.h"

static struct termios initial_settings, new_settings;
static int peek_character = -1;

#define SIZER 64
#define SIZEC 32

using namespace std;
using namespace chrono;

random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> dis(0, 255);

#define INF 1e9
int push = 0, push_size = 5;
char back[SIZER][SIZEC];

volatile bool interrupt_received = false;
static void InterruptHandler(int signo)
{
    interrupt_received = true;
}

// 색 처리 수정 필요
int sock;
char IP[30] = "192.168.1.3";
int port;
int p1[3];
int p2[3];

class Map
{
public:
    void print()
    {
        if (push == 0)
        {

            while (send(sock, back, sizeof(back), 0) == -1)
            {
                sock = sock_init(IP, port);
                sleep(1);
            }

            send(indata[4], p1, sizeof(p1), 0);
            send(indata[5], p2, sizeof(p2), 0);
        }
        push++;
        push %= push_size;
    }

    void flush()
    {
        memset(back, 0, sizeof back);
    }
};

class Effect
{
    int Duration;
    const int Slow = 50;
    pair<int, int> shape[50];
    int shapeCnt = 0;

public:
    int x, y;
    int liveTime;
    int color;
    bool live = 0;
    Effect(int x, int y, int c, int d = 250) : x(x), y(y), color(c), Duration(d) {}
    Effect() {}
    void init(int a, int b, int c, int d = 250)
    {
        Duration = d;
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
                back[p.second][p.first] = color;
            }
            return;
        }

        int ny, tmp;
        shapeCnt = 0;
        for (int i = 0; i < SIZEC; i++)
        {
            if (liveTime / Slow > abs(x - i))
            {
                if (liveTime / Slow * liveTime / Slow - (x - i) * (x - i) < 0)
                    continue;
                tmp = (int)sqrt(liveTime / Slow * liveTime / Slow - (x - i) * (x - i));
                ny = -tmp + y;
                if (ny >= 0 && ny < SIZER)
                {
                    shape[shapeCnt++] = {i, ny};
                }
                ny = tmp + y;
                if (ny >= 0 && ny < SIZER)
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
    const int movingCycle = 28;

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

        return y < SIZER - 1 && y > 0;
    }

    void draw()
    {
        back[y][x] = tag * 10 + 3;
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

        return y < SIZER - 1 && y > 0;
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

    void get_effect(int x, int y, int col, int d = 250)
    {
        for (auto &p : efectPool)
        {
            if (!p.live)
            {
                p.init(x, y, col, d);
                return;
            }
        }
        efectPool.push_back(Effect(x, y, col, d));
    }

} objectpool;

class Boom
{
    const int Duration = 2000;
    const int Slow = 20;
    pair<int, int> shape[50];
    int shapeCnt = 0;

public:
    const int Cooltime = 7000;

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
                back[p.second][p.first] = 30;
            }
            return;
        }

        int ny, tmp;
        shapeCnt = 0;
        for (int i = 0; i < SIZEC; i++)
        {
            if (liveTime / Slow > abs(x - i))
            {
                tmp = (int)sqrt(liveTime / Slow * liveTime / Slow - (x - i) * (x - i));
                ny = -tmp + y;
                if (ny >= 0 && ny < SIZER)
                {
                    shape[shapeCnt++] = {i, ny};
                }
                ny = tmp + y;
                if (ny >= 0 && ny < SIZER)
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
    const int Duration = 600;
    vector<pair<int, int>> shape = {{-3, 0}, {-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {0, -2}, {1, -2}, {1, 2}, {2, -1}, {3, 0}, {2, 1}, {2, 2}, {-4, 1}, {-4, 2}, {4, 1}, {4, 2}};

public:
    const int Cooltime = 7000;

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
            back[(y + p.second * rev + SIZER) % SIZER][(x + p.first + SIZEC) % SIZEC] = tag * 10 + 2;
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
    const double speed = 0.035;
    const int shootCooltime = 299;
    const int ghostTime = 30;
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
            back[(y + p.second * rev + SIZER) % SIZER][(x + p.first + SIZEC) % SIZEC] = tag * 10 + 1;
        }
    }

    void erase()
    {
        for (auto &p : shape)
        {
            back[(y + p.second * rev + SIZER) % SIZER][(x + p.first + SIZEC) % SIZEC] = 0;
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
        objectpool.get(x, y + 2 * -rev, 0, -rev, 10, tag);
    }
    void shoot2()
    {
        if (shootCooldown > 0)
        {
            return;
        }
        shootCooldown = shootCooltime;
        objectpool.get(x, y + 2 * -rev, -1, -rev, 2, tag);
        objectpool.get(x, y + 2 * -rev, 1, -rev, 2, tag);
        objectpool.get(x, y + 2 * -rev, 0, -rev, 2, tag);
    }
    void shoot3()
    {
        if (shootCooldown > 0)
        {
            return;
        }
        shootCooldown = shootCooltime * 7;
        objectpool.get(x - 1, y + 2 * -rev, 0, -rev * 3, 7, tag);
        objectpool.get(x + 1, y + 2 * -rev, 0, -rev * 3, 7, tag);
        objectpool.get(x, y + 2 * -rev, 0, -rev * 3, 7, tag);
        objectpool.get(x, y + 1 * -rev, 0, -rev * 3, 7, tag);
        objectpool.get(x, y + 3 * -rev, 0, -rev * 3, 7, tag);

        objectpool.get(x + 2, y + 3 * -rev, 0, -rev * 3, 7, tag);
        objectpool.get(x - 2, y + 3 * -rev, 0, -rev * 3, 7, tag);
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
                objectpool.get_effect(b.x, b.y, 30);
                b.live = 0;
                continue;
            }

            if (p->shield->collision(b.x, b.y))
            {
                objectpool.get_effect(b.x, b.y, b.tag * 10 + 5, 220);
                b.live = 0;
            }
            else if (p->collision(b.x, b.y))
            {
                objectpool.get_effect(b.x, b.y, b.tag * 10 + 5, 220);
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
                objectpool.get_effect(player2->x, player2->y, 30);
                player2->Ghost();
            }
        }

        if (player2->boom->collision(player1->x, player1->y))
        {
            if (player1->hit(player2->boom->damage))
            {
                objectpool.get_effect(player1->x, player1->y, 30);
                player1->Ghost();
            }
        }

        draw();
    }
};

class GameManager
{
public:
    ObjectManager objectManager;
    Map map;

    void key_process1()
    {
        int joy = indata[0];
        int button1 = indata[2];

        switch (joy)
        {
        case 2:
            objectManager.player1->move_left();
            break;
        case 3:
            objectManager.player1->move_right();
            break;
        case 0:
            objectManager.player1->change_mod();
            break;
        case 1:
            objectManager.player1->use_shield();
            break;
        }

        if (button1 == 0)
            objectManager.player1->use_boom();
    }

    void key_process2()
    {

        int joy = indata[1];
        int button1 = indata[3];

        switch (joy)
        {
        case 3:
            objectManager.player2->move_left();
            break;
        case 2:
            objectManager.player2->move_right();
            break;
        case 0:
            objectManager.player2->change_mod();
            break;
        case 1:
            objectManager.player2->use_shield();
            break;
        }

        if (button1 == 0)
            objectManager.player2->use_boom();
    }
    // 체력 붐 쉴드
    bool update()
    {
        map.flush();
        objectManager.cycle();
        p1[0] = objectManager.player1->hp;
        p2[0] = objectManager.player2->hp;
        p1[1] = objectManager.player1->boomCooldown, p2[1] = objectManager.player2->boomCooldown;
        p1[2] = objectManager.player1->shieldCooldown, p2[2] = objectManager.player2->shieldCooldown;
        return objectManager.player1->hp > 0 && objectManager.player2->hp > 0;
    }
    void rander()
    {
        map.print();
    }

    void start()
    {
        objectManager.player1 = new Player(16, 58, 500, 1);
        objectManager.player2 = new Player(16, 5, 500, 2);

        auto duration = microseconds(1000);
        while (1)
        {
            system_clock::time_point start_time = system_clock::now();
            key_process1();
            key_process2();
            if (!update())
                break;
            rander();

            system_clock::time_point end_time = system_clock::now();
            microseconds micro = duration_cast<microseconds>(end_time - start_time);
            this_thread::sleep_for(duration - micro);
        }

        finish();
    }

    void finish()
    {
        int total = 0;
        auto duration = microseconds(1000);
        int cycle = 100;
        objectManager.player2->shootCooldown = 10000000;
        objectManager.player1->shootCooldown = 10000000;

        if (objectManager.player1->hp > 0)
        {
            while (1)
            {
                if (total == 11000)
                    break;
                system_clock::time_point start_time = system_clock::now();

                if (cycle == 100 && total < 8000)
                {
                    objectpool.get_effect(objectManager.player2->x, objectManager.player2->y, 30, 2000);
                    cycle = 0;
                }

                update();

                if (total >= 9000)
                    objectManager.player2->erase();
                rander();
                total++;
                cycle++;
                system_clock::time_point end_time = system_clock::now();
                microseconds micro = duration_cast<microseconds>(end_time - start_time);
                this_thread::sleep_for(duration - micro);
            }
        }
        else
        {
            while (1)
            {
                if (total == 11000)
                    break;

                system_clock::time_point start_time = system_clock::now();

                if (cycle == 100 && total < 8000)
                {
                    objectpool.get_effect(objectManager.player1->x, objectManager.player1->y, 30, 2000);
                    cycle = 0;
                }
                update();
                if (total >= 9000)
                    objectManager.player1->erase();
                rander();
                total++;
                cycle++;

                system_clock::time_point end_time = system_clock::now();
                microseconds micro = duration_cast<microseconds>(end_time - start_time);
                this_thread::sleep_for(duration - micro);
            }
        }
    }
};

GameManager gm;

int main(int argc, char *argv[])
{
    int option = 1;
    pthread_t tid, tid2;
    int thr_id = pthread_create(&tid, NULL, t_function_main, &option);

    port = 9000;
    sock = sock_init(IP, port);
    init_keyboard();

    gm.start();

    pthread_join(tid, NULL);

    return 0;
}
