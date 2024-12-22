# 시스템 프로그래밍 및 실습 프로젝트

- 주제: 갤러그 스타일 PvP 기반 슈팅 게임
- 조원:
    - 김현빈
    - 신지원
    - 이종학
    - 이휘수


## How to Run

- client1
    ```
    cd ./client1
    make
    ./controller
    ```

- client2
    ```
    cd ./client2
    make
    ./controller
    ```

- server
    ```
    cd ./server
    make
    ./galaga
    ```

- led
    ```
    cd ./led
    make
    sudo ./galaga_program/galaga --led-cols=64 --led-rows=32
    ```