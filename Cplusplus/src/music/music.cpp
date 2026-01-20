// https://www.bilibili.com/video/BV1XBBKBNEao
// 遠い空へ
#include <Windows.h>

#include <atomic>
#include <iostream>
#pragma comment(lib, "winmm.lib")

std::atomic<bool> start_play { false };

// 音符映射表

enum Scale
{

  Rest = 0,

  C8  = 108,
  B7  = 107,
  A7s = 106,
  A7  = 105,
  G7s = 104,
  G7  = 103,
  F7s = 102,
  F7  = 101,
  E7  = 100,

  D7s = 99,
  D7  = 98,
  C7s = 97,
  C7  = 96,
  B6  = 95,
  A6s = 94,
  A6  = 93,
  G6s = 92,
  G6  = 91,
  F6s = 90,
  F6  = 89,

  E6  = 88,
  D6s = 87,
  D6  = 86,
  C6s = 85,
  C6  = 84,
  B5  = 83,
  A5s = 82,
  A5  = 81,
  G5s = 80,
  G5  = 79,
  F5s = 78,

  F5  = 77,
  E5  = 76,
  D5s = 75,
  D5  = 74,
  C5s = 73,
  C5  = 72,
  B4  = 71,
  A4s = 70,
  A4  = 69,
  G4s = 68,
  G4  = 67,

  F4s = 66,
  F4  = 65,
  E4  = 64,
  D4s = 63,
  D4  = 62,
  C4s = 61,
  C4  = 60,
  B3  = 59,
  A3s = 58,
  A3  = 57,
  G3s = 56,

  G3  = 55,
  F3s = 54,
  F3  = 53,
  E3  = 52,
  D3s = 51,
  D3  = 50,
  C3s = 49,
  C3  = 48,
  B2  = 47,
  A2s = 46,
  A2  = 45,

  G2s = 44,
  G2  = 43,
  F2s = 42,
  F2  = 41,
  E2  = 40,
  D2s = 39,
  D2  = 38,
  C2s = 37,
  C2  = 36,
  B1  = 35,
  A1s = 34,

  A1  = 33,
  G1s = 32,
  G1  = 31,
  F1s = 30,
  F1  = 29,
  E1  = 28,
  D1s = 27,
  D1  = 26,
  C1s = 25,
  C1  = 24,
  B0  = 23,

  A0s = 22,
  A0  = 21

};

enum Voice
{

  X1 = C2,
  X2 = D2,
  X3 = E2,
  X4 = F2,
  X5 = G2,
  X6 = A2,
  X7 = B2,

  L1 = C3,
  L2 = D3,
  L3 = E3,
  L4 = F3,
  L5 = G3,
  L6 = A3,
  L7 = B3,

  M1 = C4,
  M2 = D4,
  M3 = E4,
  M4 = F4,
  M5 = G4,
  M6 = A4,
  M7 = B4,

  H1 = C5,
  H2 = D5,
  H3 = E5,
  H4 = F5,
  H5 = G5,
  H6 = A5,
  H7 = B5,

  lows_speed   = 1000,
  middle_speed = 667,
  high_speed   = 400,

  _ = 0XFF

};

void printing(int num)
{
  for (int i = 0; i <= num - 30; i++)

    std::cout << ">";

  std::cout << num << std::endl;
}

// 音乐播放函数

void soundtrack1()
{
  HMIDIOUT handle;

  // 打开默认MIDI输出设备（设备ID=0，无回调函数）

  midiOutOpen(&handle, 0, 0, 0, CALLBACK_NULL);

  int instrument = 0;

  DWORD programMsg = (instrument << 8) | 0xC0;

  midiOutShortMsg(handle, programMsg);

  int volume = 0x7f;  // 音量

  int voice = 0x0;  // 存储MIDI音符消息

  int sleep = 400;  // 初始音符持续时间（单位：毫秒）

  int tmp = 8;  // 音高偏移量（可以用来微调

  int sta = middle_speed - 50;

  // 音符规则：

  // 0，00，000，休止符，分别为四分之一，二分之一，全音

  // 222，444，666，后面的内容是和弦（原理是速度够快就分不出是单独发音还是同时发音

  // 1，2，4，8，16，分别为四分，八分...以此类推11，111反之

  // 0x开头的是总体改变速度的标记

  int qp [] {

    1,   000,       16,      M3,  M2,        1,         M3,      111, M6,
    8,   M3,        1,       M5,  111,       M2,        16,      M1,  M1,

    16,  M3,        M2,      1,   M3,        11,        M6,      1,   H1,
    111, H5,        2,       M7,  H1,        M7,        M5,      M2,  M3,
    11,  M5,        0xff'f1, 1,   L6,        L5,        444,     L3,  X4,
    11,  L6,        1,       M3,  444,       L5,        L2,      1,   L7,
    0,   2,         M1,      L7,  L5,        222,       L4,      10,  L6,
    2,   L7,        M1,      M2,  444,       L7,        X7,      2,   M5,
    M6,  11,        M5,      444, L3,        X4,        11,      L6,  1,
    M3,  444,       L5,      X3,  1,         M5,        L7,      L5,  222,
    L3,  X2,        1,       L6,  1,         X6,        222,     L2,  1,
    M2,  444,       G3s,     X7,  1,         M2,        M3,      L7,  444,
    L3,  X4,        1,       L6,  2,         X1,        X4,      222, L1,
    1,   M3,        444,     M2,  X3,        2,         L7,      2,   B1,
    X3,  2,         M1,      2,   L7,        L5,        444,     L4,  L1,
    2,   L6,        2,       A1,  X2,        2,         L7,      M1,  M2,
    444, M3,        L7,      2,   M5,        2,         M6,      222, X3,
    1,   M5,        1,       X5,  444,       L3,        L1,      1,   L6,
    2,   X1,        X4,      222, L1,        1,         M3,      444, M2,
    L6,  1,         M5,      L7,  L5,        444,       L3,      L2,  2,
    L6,  2,         X3,      X6,  2,         L5,        1,       L7,  444,
    L2,  A1,        111,     L6,  0xf'ff'1c, 0xff'ff,   1,       M3,  M5,
    444, L4,        X4,      1,   M6,        2,         L1,      444, M1,
    2,   L6,        2,       M5,  M6,        444,       L3,      X3,  1,
    M5,  444,       L5,      2,   L5,        L7,        1,       M2,  444,
    L5,  X5,        1,       M3,  444,       M2,        L6,      1,   M5,
    L7,  444,       L3,      X3,  1,         M1,        2,       X6,  L5,

    2,   M1,        M2,      444, L1,        X2,        1,       M3,  2,
    X6,  444,       L3,      L2,  2,         L5,        M2,      M3,  444,
    L2,  X3,        1,       M2,  2,         X7,        444,     L5,  L2,
    2,   L7,        2,       M1,  M2,        444,       L4,      X4,  1,
    M3,  2,         L1,      444, M1,        L6,        1,       M4,  M5,

    G4s, 8,         G2s,     X7,  L2,        L3,        4,       M6,  8,
    L2,  L5,        L7,      M2,  M3,        4,         M7,      8,   L5,
    L7,  M2,        M3,      G4s, 444,       L4,        X4,      1,   M6,
    2,   L1,        444,     L6,  L3,        2,         M1,      2,   M5,
    M6,  444,       L2,      X3,  1,         M5,        2,       X7,  444,
    L5,  L2,        2,       L7,  1,         M2,        444,     L5,  X5,
    1,   M3,        2,       L2,  222,       L5,        1,       M5,  2,
    M2,  1,         M7,      444, L1,        X3,        1,       H1,  2,
    X6,  444,       L5,      L3,  1,         H2,        H3,      444, L1,
    X2,  1,         H3,      2,   X6,        L4,        444,     L6,  2,
    H2,  H1,        444,     L2,  X3,        1,         M7,      2,   X7,
    L2,  444,       L5,      2,   M5,        M7,        444,     L4,  X4,
    1,   H1,        2,       L1,  L6,        0xf'ff'fc, 0xff'f1, 444, M2,
    M1,  2,         M7,      444, M1,        2,         M6,      444, L7,
    L2,  1,         G4s,     2,   X7,        444,       M1,      L2,  1,
    M6,  2,         G3s,     444, M2,        G2s,       1,       M7,  1,
    444, L4,        X4,      1,   H3,        2,         L1,      444, M1,
    L6,  2,         H2,      H1,  444,       M2,        L2,      1,   M7,
    444, L7,        L2,      1,   M5,        444,       L5,      1,   M3,
    444, M1,        X6,      111, M6,        0xf'ff'1c, 444,     X4,  2,
    H3,  M6,        H1,      M6,  H1,        M6,        H1,      M6,  H2,
    M6,  H1,        M6,      444, X3,        2,         H3,      M6,  H1,
    M6,  H1,        M6,      H1,  M6,        H2,        M6,      H1,  M6,

    444, X4,        4,       H3,  8,         L1,        L3,      L6,  M1,
    M6,  M1,        2,       H1,  M6,        H1,        M6,      H1,  M6,
    H2,  M6,        H1,      M6,  444,       X3,        4,       H3,  8,
    X3,  X6,        L2,      L5,  2,         H1,        M6,      H1,  M6,
    H2,  M6,        G6,      M6,  H2,        M6,        0xff'f1, 444, M3,
    1,   M6,        2,       L6,  M1,        444,       M1,      1,   H3,
    444, M2,        1,       M7,  2,         L6,        444,     L5,  2,
    H1,  444,       L7,      2,   M7,        444,       L5,      2,   M5,
    444, M3,        1,       M6,  2,         M1,        444,     L6,  2,
    M7,  444,       M1,      2,   H1,        444,       L6,      2,   H2,
    444, M2,        2,       H5,  444,       L5,        2,       H6,  444,
    L3,  1,         H5,      2,   L7,        L5,        444,     M3,  1,
    M6,  2,         L6,      M1,  444,       M1,        1,       H3,  444,
    M2,  1,         H5,      444, L3,        1,         M7,      444, L7,
    1,   M5,        444,     M3,  1,         M6,        2,       M1,  L6,

    444, M3,        1,       M6,  2,         L6,        M1,      444, M1,
    1,   H3,        444,     L3,  2,         M7,        2,       L5,  L7,
    444, L5,        2,       H1,  444,       L7,        2,       M7,  444,
    L5,  2,         M5,      444, M3,        2,         M6,      2,   L6,
    M1,  444,       L6,      2,   M7,        444,       M1,      2,   H1,
    444, L6,        2,       H2,  444,       M2,        2,       H5,  444,
    L5,  2,         H6,      444, L3,        1,         H5,      2,   L7,
    L5,  444,       M3,      1,   M6,        2,         L6,      M1,  444,
    M1,  1,         H3,      444, L3,        1,         H5,      444, L7,
    1,   M7,        444,     M2,  1,         M5,        444,     M3,  1,
    M6,  2,         L6,      444, L6,        2,         M5,      444, M1,
    1,   M7,        444,     M1,  1,         M6,        2,       L6,  M2,
    M1,  L6,        444,     H1,  M5,        8,         H3,      8,   X6,
    L1,  L3,        L5,      L6,  444,       H3,        H1,      8,   H5,
    8,   L3,        L5,      L6,  M1,        M3,        444,     H5,  H3,
    8,   C6,        8,       L6,  M1,        M3,        M5,      M6,  H1,
    M6,  M5,        M3,      M1,  L6,        444,       L3,      1,   M3,
    444, L5,        1,       M5,  444,       M1,        L6,      1,   M6,
    2,   X3,        X6,      2,   M5,        M6,        11,      444, L7,
    L5,  1,         M5,      2,   X3,        X5,        444,     M5,  X3,
    1,   M2,        444,     L5,  1,         M3,        444,     L7,  1,
    M5,  444,       L5,      1,   L7,        444,       L5,      L3,  1,
    M1,  2,         X6,      L1,  444,       L5,        X6,      2,   M1,
    M2,  444,       M1,      L6,  1,         M3,        2,       X1,  X4,
    2,   444,       X1,      2,   M2,        444,       A1,      2,   M3,
    444, L7,        L5,      1,   M2,        2,         X2,      X5,  444,
    L5,  2,         M1,      M2,  444,       M1,        L6,      1,   M3,
    444, L6,        1,       M4,  444,       L6,        1,       M5,  444,
    M3,  L7,        X3,      4,   G4s,       8,         G2s,     X7,  L2,
    L3,  G3s,       444,     M2,  L7,        X7,        4,       M6,  8,
    X3,  L3,        L5,      L7,  M2,        M3,        444,     M5,  L7,
    4,   M7,        8,       L3,  L5,        L7,        M2,      M3,  G4s,

    444, M4,        M1,      X4,  1,         M6,        2,       X3,  X6,

    444, X3,        2,       M5,  M6,        444,       M2,      L7,  X3,
    1,   M5,        2,       X2,  X5,        444,       L7,      1,   M2,
    444, L7,        L5,      1,   M3,        444,       L7,      L5,  1,
    M5,  444,       M2,      L7,  1,         M7,        444,     M6,  M3,
    1,   H1,        444,     M6,  M3,        1,         H2,      444, M6,
    M3,  1,         H3,      444, M6,        M4,        1,       H3,  2,
    X1,  X4,        2,       H2,  H1,        444,       M5,      M2,  1,
    M7,  2,         X2,      X5,  2,         M5,        M7,      444, M6,
    M3,  1,         H1,      2,   X3,        X6,        2,       H1,  M7,
    M6,  666,       M3,      L7,  E1,        1,         G4s,     666, M3,
    L7,  X2,        1,       M6,  666,       M5,        M3,      G2s, 1,
    M7,  666,       L6,      X4,  E1,        1,         H3,      2,   X3,
    X6,  H2,        H1,      444, M2,        E1,        1,       M7,  444,
    L7,  X3,        1,       M5,  666,       L5,        X7,      1,   M3,
    2,   L5,        L3,      L1,  666,       M3,        M1,      X6,  111,
    M6,  0xf'ff'1c, 111,     L6,  L7,        1,         M1,      H1,  M7,
    M5,  M3,        M2,      111, M1,        10,        H3,      2,   H3,
    H2,  H3,        1,       H5,  M7,        M5,        10,      H3,  2,
    H3,  H2,        H6,      H7,  C6,        H7,        H5,      1,   H2,
    111, H3,        1,       H2,  H1,        M7,        M5,      111, M3,
    16,  M3,        M1,      M3,  111,       M6,

  };

  for (auto i : qp) {
    if (i == 4) {
      sleep = sta / 3;
      continue;
    }

    if (i == 2) {
      sleep = sta / 2;
      continue;
    }

    if (i == 1) {
      sleep = sta;
      continue;
    }

    if (i == 10) {
      sleep = sta * 1.5;
      continue;
    }

    if (i == 8) {
      sleep = sta / 8;
      continue;
    }

    if (i == 16) {
      sleep = sta / 16;
      continue;
    }

    if (i == 222) {
      sleep = sta / 40;
      continue;
    }

    if (i == 444) {
      sleep = sta / 70;
      continue;
    }

    if (i == 666) {
      sleep = sta / 90;
      continue;
    }

    if (i == 11) {
      sleep = sta * 2;
      continue;
    }

    if (i == 111) {
      sleep = sta * 3;
      continue;
    }

    if (i == 0) {
      Sleep(sta / 4);
      continue;
    }

    if (i == 00) {
      Sleep(sta / 2);
      continue;
    }

    if (i == 000) {
      Sleep(sta);
      continue;
    }

    if (i == 0xff'f1) {
      sta -= 25;
      continue;
    }

    if (i == 0xff'ff) {
      sta -= 50;
      continue;
    }

    if (i == 0xf'ff'1c) {
      sta += 25;
      continue;
    }

    if (i == 0xf'ff'fc) {
      sta += 50;
      continue;
    }

    // 发送MIDI消息

    voice = (volume << 16) + ((i + tmp) << 8) + 0x90;

    midiOutShortMsg(handle, voice);

    printing(i);

    Sleep(sleep);  // 等待音符持续时间
  }

  midiOutClose(handle);
}

int main()
{
  std::cout << "要开始播放了！\n";

  Sleep(2000);

  soundtrack1();

  return 0;
}
