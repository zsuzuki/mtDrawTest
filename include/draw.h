//
// Copyright 2023 Suzuki Yoshinori(wave.suzuki.z@gmail.com)
// since: 26 Apr 2023
//
#pragma once

#include <array>
#include <cinttypes>
#include <utility>

constexpr int ScreenWidth  = 1024;
constexpr int ScreenHeight = 768;
constexpr int PointSize    = 24;

void CreateWindow(const char* title, bool withJoystick = false);
void DestroyWindow();
void FrameUpdate();
void FrameSync();

void SetColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);
void FillRect(int x, int y, int w, int h);
void DrawRect(int x, int y, int w, int h);
void DrawPoint(int x, int y);
void Print(int x, int y, const char* msg);
template <class... Args>
void
Printf(int x, int y, const char* fmt, Args&&... args)
{
    std::array<char, 256> msg;
    snprintf(msg.data(), msg.size(), fmt, std::forward<Args>(args)...);
    Print(x, y, msg.data());
}
