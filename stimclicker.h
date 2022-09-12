#ifndef STIMCLICKER_H
#define STIMCLICKER_H

#pragma once

#include <uiautomationclient.h>

class StimClicker {
    HWND btn1, btn2;
    HWND led1, led2;

public:
    HANDLE processHandle;

    StimClicker();
    ~StimClicker();
    void sendClicks();
    bool isStimON();
};

#endif // STIMCLICKER_H
