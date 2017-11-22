/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: nexor
 *
 * Created on November 22, 2017, 4:34 PM
 */
#include<sys/ioctl.h>
#include<fcntl.h>
#include<linux/kd.h>
#include <cstdlib>
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<string.h>
#include<time.h>
#include<unistd.h>
#include<thread>

using namespace std;

static int* GetPixelAt(int x, int y) {
    XColor c;
    Display *d = XOpenDisplay((char *) NULL);
    XImage *image;
    image = XGetImage(d, RootWindow(d, DefaultScreen(d)), x, y, 1, 1, AllPlanes, XYPixmap);
    c.pixel = XGetPixel(image, 0, 0);
    XFree(image);
    XQueryColor(d, DefaultColormap(d, DefaultScreen(d)), &c);
    int* color = new int[3] {
        c.red / 256, c.green / 256, c.blue / 256
    };
    XCloseDisplay(d);
    return color;
}

static int* GetMousePos() {
    int number_of_screens;
    int i;
    Bool result;
    Window *root_windows;
    Window window_returned;
    int root_x, root_y;
    int win_x, win_y;
    unsigned int mask_return;

    Display *display = XOpenDisplay(NULL);
    number_of_screens = XScreenCount(display);
    root_windows = (Window*) malloc(sizeof (Window) * number_of_screens);
    for (i = 0; i < number_of_screens; i++) {
        root_windows[i] = XRootWindow(display, i);
    }
    for (i = 0; i < number_of_screens; i++) {
        result = XQueryPointer(display, root_windows[i], &window_returned,
                &window_returned, &root_x, &root_y, &win_x, &win_y,
                &mask_return);
        if (result == True) {
            break;
        }
    }
    free(root_windows);
    XCloseDisplay(display);
    return new int[2] {
        root_x, root_y
    };

}

static int is_same_color(int* color1, int* color2) {
    return color1[0] == color2[0] && color1[1] == color2[1] && color1[2] == color2[2];
}

void mouseClick(int x, int y) {
    Display *d = XOpenDisplay(0);
    Window root = DefaultRootWindow(d);
    int* origin = GetMousePos();
    XWarpPointer(d, None, root, 0, 0, 0, 0, x, y);
    XFlush(d);
    XEvent event;
    if (d == NULL) {
        fprintf(stderr, "Cannot initialize the display\n");
        exit(EXIT_FAILURE);
    }
    memset(&event, 0x00, sizeof (event));
    event.type = ButtonPress;
    event.xbutton.button = Button1;
    event.xbutton.same_screen = True;
    XQueryPointer(d, RootWindow(d, DefaultScreen(d)), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
    event.xbutton.subwindow = event.xbutton.window;
    while (event.xbutton.subwindow) {
        event.xbutton.window = event.xbutton.subwindow;
        XQueryPointer(d, event.xbutton.window, &event.xbutton.root, &event.xbutton.subwindow, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
    }
    if (XSendEvent(d, PointerWindow, True, 0xfff, &event) == 0) fprintf(stderr, "Error\n");
    XFlush(d);
    usleep(100000);
    event.type = ButtonRelease;
    event.xbutton.state = 0x100;
    if (XSendEvent(d, PointerWindow, True, 0xfff, &event) == 0) fprintf(stderr, "Error\n");
    XFlush(d);
    usleep(200);
    XWarpPointer(d, None, root, 0, 0, 0, 0, origin[0], origin[1]);
    XFlush(d);
    XCloseDisplay(d);
}

void beep() {
    int freq = 2000; // freq in hz
    int len = 1000; // len in ms

    int fd = open("/dev/console", O_WRONLY);
    ioctl(fd, KIOCSOUND, (int) (1193180 / freq));
    usleep(1000 * len);
    ioctl(fd, KIOCSOUND, 0);
    close(fd);
}

static void tryAndPickMid() {
    while (true) {
        //        int* color = GetPixelAt(1659, 943);
        //        printf("%d,%d,%d", color[0], color[1], color[2]);
        //        int* m_pos = GetMousePos();
        //        cout << m_pos[0] << " " << m_pos[1] << endl;
        int* radiant_pos = new int[2] {
            1616, 965
        };
        int* dire_pos = new int[2] {
            1659, 943
        };
        int* team_color_radiant = new int[3] {
            13, 178, 28
        };
        int* team_color_dire = new int[3] {
            12, 213, 27
        };
        //        int* dire_pos = new int[2] {1616, 965};
        int* radiant_color = GetPixelAt(radiant_pos[0], radiant_pos[1]);
        int* dire_color = GetPixelAt(dire_pos[0], dire_pos[1]);
        printf("%d,%d,%d\n", radiant_color[0], radiant_color[1], radiant_color[2]);
        //        cout << color[0] << ", " << color[1] << ", " << color[2];
        if (is_same_color(radiant_color, team_color_radiant)) { // should approximate on eventual failure ?
            mouseClick(radiant_pos[0], radiant_pos[1]);
            break;
        } else if (is_same_color(dire_color, team_color_dire)) {
            mouseClick(dire_pos[0], dire_pos[1]);
            break;
        }
    }
}

static void LogMe(const char * fmt, ...) {
    va_list va_alist;
    char buf[256], logbuf[256];
    FILE* file;
    struct tm* current_tm;
    time_t current_time;
    time(&current_time);
    current_tm = localtime(&current_time);
    sprintf(logbuf, "%02d:%02d:%02d-> ", current_tm->tm_hour, current_tm->tm_min, current_tm->tm_sec);
    va_start(va_alist, fmt);
    vsprintf(buf, fmt, va_alist);
    va_end(va_alist);
    strcat(logbuf, buf);
    strcat(logbuf, "\n");

    if ((file = fopen("key.log", "a+")) != NULL) {
        fputs(logbuf, file);
        fclose(file);
    }
}

static int listen_for_keys() {
    Display * display;
    char szKey[32];
    char szKeyOld[32] = {0};
    char szBit;
    char szBitOld;
    int iCheck;
    char szKeysym;
    char * szKeyString;
    int iKeyCode;
    Window focusWin = NULL;
    int iReverToReturn = NULL;
    display = XOpenDisplay(NULL);
    if (display == NULL) {
        return -1;
    }
    while (true) {
        XQueryKeymap(display, szKey);
        if (memcmp(szKey, szKeyOld, 32) != NULL) {
            for (int i = 0; i < sizeof ( szKey); i++) {
                szBit = szKey[i];
                szBitOld = szKeyOld[i];
                iCheck = 1;
                for (int j = 0; j < 8; j++) {
                    if ((szBit & iCheck) && !(szBitOld & iCheck)) {
                        iKeyCode = i * 8 + j;
                        szKeysym = XKeycodeToKeysym(display, iKeyCode, 0);
                        szKeyString = XKeysymToString(szKeysym);
                        XGetInputFocus(display, &focusWin, &iReverToReturn);
                        printf("%d", iKeyCode);
                        fflush(stdout);
                        switch (iKeyCode) {
                            case 91:
                                cout << '!';
                                thread t(tryAndPickMid);
                                t.detach();
                                break;
                        }
                    }
                    iCheck = iCheck << 1;
                }
            }
        }
        memcpy(szKeyOld, szKey, 32);
    }
    XCloseDisplay(display);
}

int main(int argc, char** argv) {
    listen_for_keys();
}

