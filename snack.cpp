#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <Wire.h>


MD_Parola P = MD_Parola(11, 13, 10, 2);
MD_MAX72XX mx = MD_MAX72XX(11, 13, 10, 2);

volatile int w = 8;
volatile int h = 8;

volatile int snack_body[64][2] = {};
volatile bool snack[8][8] = {};
volatile int food[2] = {4, 6};
volatile int snack_head[2] = {4, 3};

volatile uint8_t bitmap[8] = {};

volatile int score = 3;
volatile int direction = 1;
volatile int old_direction = 1;

volatile bool game_over = false;
volatile long long time_;
volatile int button_ = 0;

const uint8_t game_over_bitmap[8] = {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81,};

const uint8_t b0[8] = {0x1f, 0x11, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00};
const uint8_t b1[8] = {0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,};
const uint8_t b2[8] = {0x1d, 0x15, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00,};
const uint8_t b3[8] = {0x15, 0x15, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,};
const uint8_t b4[8] = {0x07, 0x04, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,};
const uint8_t b5[8] = {0x17, 0x15, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x00,};
const uint8_t b6[8] = {0x1f, 0x15, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x00,};
const uint8_t b7[8] = {0x01, 0x01, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,};
const uint8_t b8[8] = {0x1f, 0x15, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,};
const uint8_t b9[8] = {0x17, 0x15, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00,};

int mylist[] = {b0, b1, b2, b3, b4, b5, b6, b7, b8, b9};

int sn(int n, int m);

void show();

void show_score();

void reset();

void snack_next();

void control();

void food_summon();


// 次幂
int sn(int n, int m) {
    int res = 1;
    for (int i = 0; i < m; i++) {
        res *= n;
    }
    return res;
}

// 显示当前分数
void show_score() {
    if (score >= 10) {
        int n2 = int(score / 10);
        int n1 = score - n2 * 10;

        mx.setBuffer(15, 3, mylist[n2]);
        mx.setBuffer(10, 3, mylist[n1]);
    } else {
        mx.setBuffer(15, 3, mylist[score]);
    }
}

// 显示蛇身, 食物
void show() {
    for (int i = 0; i < w; i++) {
        bitmap[i] = 0;
    }
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            snack[i][j] = false;
        }
    }

    for (int i = 0; i < h * w; i++) {
        snack[snack_body[i][0] - 1][snack_body[i][1] - 1] = true;
    }
    snack[food[0] - 1][food[1] - 1] = true;

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            if (snack[i][j]) {
                bitmap[j] = bitmap[j] + sn(2, i);
            }
        }
    }

    show_score();
    mx.setBuffer(7, 8, bitmap);
}

// 判断蛇的下一步动作
void snack_next() {
    if (direction == 2) {
        snack_head[0] = snack_head[0] + 1;
    } else if (direction == 4) {
        snack_head[0] = snack_head[0] - 1;
    } else if (direction == 1) {
        snack_head[1] = snack_head[1] + 1;
    } else if (direction == 3) {
        snack_head[1] = snack_head[1] - 1;
    }

    if (snack_head[0] == food[0] && snack_head[1] == food[1]) {
        score++;
        snack_body[score - 1][0] = snack_head[0];
        snack_body[score - 1][1] = snack_head[1];
        food_summon();
    } else {
        for (int i = 0; i < score - 1; i++) {
            snack_body[i][0] = snack_body[i + 1][0];
            snack_body[i][1] = snack_body[i + 1][1];
        }
        snack_body[score - 1][0] = snack_head[0];
        snack_body[score - 1][1] = snack_head[1];
    }

    for (int i = 0; i < score - 1; i++) {
        if (snack_head[0] == snack_body[i][0] && snack_head[1] == snack_body[i][1]) {
            game_over = true;
        }
    }
    if (snack_head[1] == 0 || snack_head[0] == 0 ||
        snack_head[1] == w + 1 || snack_head[0] == h + 1) {
        game_over = true;
    }
}

// 控制蛇
void control() {
    time_ = millis();
    button_ = 0;
    old_direction = direction;

    bool s3 = false, s2 = false;

    while (millis() - time_ < 500) {
        if (!digitalRead(3)) {
            s3 = true;
        }
        if (!digitalRead(2)) {
            s2 = true;
        }
        if (digitalRead(3)) {
            if (button_ != 1 && s3) {
                randomSeed(millis());
                if (old_direction == 1) { direction = 4; } else { direction = old_direction - 1; }
            }
            button_ = 1;
        } else if (digitalRead(2)) {
            if (button_ != -1 && s2) {
                randomSeed(millis());
                if (old_direction == 4) { direction = 1; } else { direction = old_direction + 1; }
            }
            button_ = -1;
        }
    }
}

// 生成食物
void food_summon() {
    bool m = false;
    int x = random(1, w);
    int y = random(1, h);
    if (x == snack_head[1] && y == snack_head[0]) { m = true; }
    for (int i = 0; i < score - 1; i++) {
        if (x == snack_body[i][1] && y == snack_body[i][0]) {
            m = true;
        }
    }
    if (m) { food_summon(); } else {
        food[1] = x;
        food[0] = y;
    }
}

// 重置
void reset() {
    for (int i = 0; i < 64; i++) {
        snack_body[i][0] = 0;
        snack_body[i][1] = 0;
    }

    food[0] = 4;
    food[1] = 6;
    snack_head[0] = 4;
    snack_head[1] = 3;

    score = 3;
    direction = 1;
    old_direction = 1;

    time_ = millis();
    button_ = 0;

    game_over = false;

    snack_body[0][0] = 4;
    snack_body[0][1] = 1;
    snack_body[1][0] = 4;
    snack_body[1][1] = 2;
    snack_body[2][0] = 4;
    snack_body[2][1] = 3;
}

void setup() {
    randomSeed(55688888);
    mx.begin();
    P.begin();
    Serial.begin(9600);

    snack_body[0][0] = 4;
    snack_body[0][1] = 1;
    snack_body[1][0] = 4;
    snack_body[1][1] = 2;
    snack_body[2][0] = 4;
    snack_body[2][1] = 3;

    pinMode(2, INPUT);
    pinMode(3, INPUT);

}

void loop() {
    P.displayClear();
    mx.clear();

    show();

    control();

    snack_next();

    if (game_over) {
        for (int i = 0; i < 5; i++) {
            P.displayClear();
            mx.clear();
            delay(250);
            show();
            mx.setBuffer(15, 8, game_over_bitmap);
            delay(250);
        }
        while (true) {
            delay(250);
            if (digitalRead(3) || digitalRead(2)) {
                reset();
                break;
            }
        }
    }
}

