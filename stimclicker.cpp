#include "stimclicker.h"

#include <uiautomationclient.h>
#include <iostream>

HRESULT InitializeUIAutomation(IUIAutomation** ppAutomation)
{
    auto hr = CoInitialize(NULL);
    return CoCreateInstance(CLSID_CUIAutomation, NULL,
        CLSCTX_INPROC_SERVER, IID_IUIAutomation,
        reinterpret_cast<void**>(ppAutomation));
}

// Global Automation Object:
IUIAutomation* g_pAutomation;
auto hr = InitializeUIAutomation(&g_pAutomation);


void sendClick(HWND btn) {

    RECT rect;

    if (GetWindowRect(btn, &rect)) {

        SendMessage(btn, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM((rect.left + rect.right) / 2, (rect.bottom + rect.top) / 2));
        SendMessage(btn, WM_LBUTTONUP, 0, MAKELPARAM((rect.left + rect.right) / 2, (rect.bottom + rect.top) / 2));

    }
    else {
        std::cerr << "Error pushing button\n";
        throw std::exception();
    }

}


int ScreenX = 0;
int ScreenY = 0;
BYTE* ScreenData = 0;

void ScreenCap(HWND wHandle)
{
    HDC hScreen = GetWindowDC(wHandle);
    ScreenX = 1;
    ScreenY = 1;  // GetDeviceCaps(hScreen, VERTRES);

    HDC hdcMem = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, ScreenX, ScreenY);
    HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);
    BitBlt(hdcMem, 0, 0, ScreenX, ScreenY, hScreen, 5, 5, SRCCOPY);
    SelectObject(hdcMem, hOld);

    BITMAPINFOHEADER bmi = { 0 };
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biPlanes = 1;
    bmi.biBitCount = 32;
    bmi.biWidth = ScreenX;
    bmi.biHeight = -ScreenY;
    bmi.biCompression = BI_RGB;
    bmi.biSizeImage = 0;// 3 * ScreenX * ScreenY;

    if (ScreenData)
        free(ScreenData);
    ScreenData = (BYTE*)malloc(4 * ScreenX * ScreenY);

    GetDIBits(hdcMem, hBitmap, 0, ScreenY, ScreenData, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

    ReleaseDC(GetDesktopWindow(), hScreen);
    DeleteDC(hdcMem);
    DeleteObject(hBitmap);
}

inline int PosB(int x, int y)
{
    return ScreenData[4 * ((y * ScreenX) + x)];
}

inline int PosG(int x, int y)
{
    return ScreenData[4 * ((y * ScreenX) + x) + 1];
}

inline int PosR(int x, int y)
{
    return ScreenData[4 * ((y * ScreenX) + x) + 2];
}




StimClicker::StimClicker() {
    // Get Handles to Relevant Window Elements on Construction

    // Get Application Window:
    HWND winMain = FindWindowA("WindowsForms10.Window.8.app.0.141b42a_r8_ad1", "OPT Production Tool v1.0.0.8");

    // Get Process Handle:
    DWORD winProcessId;
    DWORD winThreadId = GetWindowThreadProcessId(winMain, &winProcessId);
    processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, winProcessId);

    // Get UI Automation main window to traverse Automation Tree:
    IUIAutomationElement* ui_winMain = nullptr;
    auto hr = g_pAutomation->ElementFromHandle(winMain, &ui_winMain);

    if (!ui_winMain) {
        throw std::exception("Inomed GUI Window could not be found... Exiting");
        return;
    }

    // Find Buttons in UI Automation Element Tree:
    IUIAutomationElementArray* ui_btnCandidates;
    IUIAutomationCondition* pCondition;
    VARIANT varProp;
    varProp.vt = VT_BSTR;
    varProp.bstrVal = SysAllocString(L"mElBtnStartSTIM");

    hr = g_pAutomation->CreatePropertyCondition(UIA_AutomationIdPropertyId, varProp, &pCondition);

    // Find Buttons:
    int number = 0;
    try{
    hr = ui_winMain->FindAll(TreeScope_Descendants, pCondition, &ui_btnCandidates);
    }
    catch (...) {}

    ui_btnCandidates->get_Length(&number);

    if (number < 2) {
        throw std::exception("Buttons not found, exiting program... Inomed GUI needs to be set up!\n");
    }

    IUIAutomationElement* uiEl_btn1 = nullptr;
    IUIAutomationElement* uiEl_btn2 = nullptr;

    hr = ui_btnCandidates->GetElement(0, &uiEl_btn1);
    hr = ui_btnCandidates->GetElement(1, &uiEl_btn2);

    UIA_HWND uiHWND_buttonStim1, uiHWND_buttonStim2;
    uiEl_btn1->get_CurrentNativeWindowHandle(&uiHWND_buttonStim1);
    uiEl_btn2->get_CurrentNativeWindowHandle(&uiHWND_buttonStim2);

    // Set Final StimButton Handle attribute:
    btn1 = (HWND)uiHWND_buttonStim1;
    btn2 = (HWND)uiHWND_buttonStim2;


    //Same procedure for LED indicators:
    varProp.bstrVal = SysAllocString(L"StimLED");
    hr = g_pAutomation->CreatePropertyCondition(UIA_AutomationIdPropertyId, varProp, &pCondition);

    IUIAutomationElementArray* ui_ledCandidates;
    number = 0;
    hr = ui_winMain->FindAll(TreeScope_Descendants, pCondition, &ui_ledCandidates);
    ui_ledCandidates->get_Length(&number);

    if (number < 2) {
        throw std::exception("LED Indicators not found, exiting program... Inomed GUI needs to be set up!");
        return;
    }

    IUIAutomationElement* uiEl_led1 = nullptr;
    IUIAutomationElement* uiEl_led2 = nullptr;

    //Get StimButton Handles:
    hr = ui_ledCandidates->GetElement(0, &uiEl_led1);
    hr = ui_ledCandidates->GetElement(1, &uiEl_led2);

    UIA_HWND uiHWND_led1, uiHWND_led2;
    uiEl_led1->get_CurrentNativeWindowHandle(&uiHWND_led1);
    uiEl_led1->get_CurrentNativeWindowHandle(&uiHWND_led2);

    led1 = (HWND)uiHWND_led1;
    led2 = (HWND)uiHWND_led2;

}


StimClicker::~StimClicker() {
    delete g_pAutomation;
}

void StimClicker::sendClicks() {

    sendClick(btn1);
    sendClick(btn2);

}


bool StimClicker::isStimON() {
    // Reads Value of LED Indicator to determine whether Stim is ON

    // LED1:
    ScreenCap((HWND)led1);
    int ledVal1 = (int)PosG(0, 0);

    //LED2:
    ScreenCap((HWND)led2);
    int ledVal2 = (int)PosG(0, 0);

    if ((ledVal1 == 255) || (ledVal2 == 255)) throw std::exception();

    if ((ledVal1<230) && (ledVal2<230)) return true;

    return false;
}
