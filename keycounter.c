#include "keycounter.h"
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <libgen.h>
#include <time.h>
#include <sys/stat.h>

#define MAX_KEY_CODE 256
#define NUM_MODIFIER_COMBINATIONS 16

int keyCounts[MAX_KEY_CODE][NUM_MODIFIER_COMBINATIONS] = {{0}}; // 16 possible states of modifier keys

time_t alarmSetTime = 0; // Global variable to store the time when setAlarm is first called

int numAffectedByCaps = sizeof(affectedByCaps) / sizeof(affectedByCaps[0]); // Calculate the number of elements in affectedByCaps
// Initialize a new boolean array with 256 elements, all set to false
bool isKeyAffectedByCaps[MAX_KEY_CODE] = {false};

CFRunLoopRef runLoopRef = NULL;
bool useJson = false;
CGEventFlags lastFlags = 0;

// Function to initialize isKeyAffectedByCaps array based on affectedByCaps
void initializeIsKeyAffectedByCaps() {
    for (int i = 0; i < numAffectedByCaps; i++) {
        int key = affectedByCaps[i];
        if (key >= 0 && key < MAX_KEY_CODE) { // Ensure the key is within bounds
            isKeyAffectedByCaps[key] = true;
        }
    }
}



// Helper function to generate a unique identifier for each key+modifier combination
// | Index | Shift | Control | Option | Command |
// |-------|-------|---------|--------|---------|
// | 0     |       |         |        |         |
// | 1     | X     |         |        |         |
// | 2     |       | X       |        |         |
// | 3     | X     | X       |        |         |
// | 4     |       |         | X      |         |
// | 5     | X     |         | X      |         |
// | 6     |       | X       | X      |         |
// | 7     | X     | X       | X      |         |
// | 8     |       |         |        | X       |
// | 9     | X     |         |        | X       |
// | 10    |       | X       |        | X       |
// | 11    | X     | X       |        | X       |
// | 12    |       |         | X      | X       |
// | 13    | X     |         | X      | X       |
// | 14    |       | X       | X      | X       |
// | 15    | X     | X       | X      | X       |
int getModifierFlagIndex(CGEventFlags flags) {
    int index = 0;
    if (flags & kCGEventFlagMaskShift) index |= 1;
    if (flags & kCGEventFlagMaskControl) index |= 2;
    if (flags & kCGEventFlagMaskAlternate) index |= 4;
    if (flags & kCGEventFlagMaskCommand) index |= 8;
    return index;
}

void _logStart(FILE *stream) {
    time_t currentTime;
    struct tm *localTime;
    char timeString[80];

    currentTime = time(NULL);
    localTime = localtime(&currentTime);
    strftime(timeString, sizeof(timeString), "%Y-%m-%dT%H:%M:%S%z", localTime);
    fprintf(stream, "[%s] ", timeString);
}
void logToNoNewLine(FILE *stream, const char *format, ...) {
    _logStart(stream);

    va_list args;
    va_start(args, format);
    vfprintf(stream, format, args);
    va_end(args);
    fflush(stream);
}

void logTo(FILE *stream, const char *format, ...) {
    _logStart(stream);

    va_list args;
    va_start(args, format);
    vfprintf(stream, format, args);
    va_end(args);

    fprintf(stream, "\n");
    fflush(stream);
}


void saveKeyCountsJson(FILE *logfile) {
    fprintf(logfile, "{\n");
    bool haveWrittenLine = false;
    for (int i = 0; i < MAX_KEY_CODE; i++) {

        // Skip keys that have not been pressed
        int total = 0;
        for (int j = 0; j < NUM_MODIFIER_COMBINATIONS; j++) {
            total += keyCounts[i][j];
        }
        if(total == 0) continue;
        if(haveWrittenLine) {
            fprintf(logfile, ",\n");
        }
        fprintf(logfile, "  \"%d\": [", i);
        for (int j = 0; j < NUM_MODIFIER_COMBINATIONS-1; j++) {
            fprintf(logfile, "%d,", keyCounts[i][j]);
        }
        fprintf(logfile, "%d]", keyCounts[i][NUM_MODIFIER_COMBINATIONS-1]);
        haveWrittenLine = true;
    }
    fprintf(logfile, "\n}\n");
}
void saveKeyCountsCsv(FILE *logfile) {
    fprintf(logfile, "\"KeyCode\",\"Shift\",\"Control\",\"ShiftControl\",\"Option\",\"ShiftOption\",\"ControlOption\",\"ShiftControlOption\",\"Command\",\"ShiftCommand\",\"ControlCommand\",\"ShiftControlCommand\",\"OptionCommand\",\"ShiftOptionCommand\",\"ControlOptionCommand\",\"ShiftControlOptionCommand\"\n");
    for (int i = 0; i < MAX_KEY_CODE; i++) {
        // Skip keys that have not been pressed
        int total = 0;
        for (int j = 0; j < NUM_MODIFIER_COMBINATIONS; j++) {
            total += keyCounts[i][j];
        }
        if(total == 0) continue;
        fprintf(logfile, "%d", i);
        for (int j = 0; j < NUM_MODIFIER_COMBINATIONS; j++) {
            fprintf(logfile, ",%d", keyCounts[i][j]);
        }
        fprintf(logfile, "\n");
    }
}

void saveKeyCounts() {
    FILE *logfile = fopen(logfileLocation, "w");
    if (!logfile) {
        fprintf(stderr, "ERROR: Unable to open log file. Ensure that you have the proper permissions.\n");
        exit(1);
    }

    if(useJson) {
        saveKeyCountsJson(logfile);
    }
    else {
        saveKeyCountsCsv(logfile);
    }
    fflush(logfile);
    fclose(logfile);
}

void timerHandler(int sig) {
    saveKeyCounts();
    alarmSetTime = 0; // Reset the stored time when the alarm goes off

    logTo(stdout, "Saved key counts");
}

void setAlarm() {
    time_t currentTime;
    time(&currentTime);

    if (alarmSetTime == 0 || (currentTime - alarmSetTime) < 5) {
        alarm(0); // Cancel any existing alarm
        alarm(1); // Set a new alarm for 1 second
        if(alarmSetTime == 0) {
            alarmSetTime = currentTime;
        }
    }
}

void siginttermHandler(int sig) {
     // Stop the current run loop
    CFRunLoopStop(CFRunLoopGetCurrent());
    // Log which signal was received
    if (sig == SIGINT) {
        logTo(stdout, "Received SIGINT");
    } else if (sig == SIGTERM) {
        logTo(stdout, "Received SIGTERM");
    }
}
void loadKeyCountsJson(FILE *logfile) {
    char line[256];
    while (fgets(line, sizeof(line), logfile) != NULL) {
        if (strstr(line, "\"") != NULL) { // Check if the line contains a key code
            int keyCode, counts[NUM_MODIFIER_COMBINATIONS];
            int itemsParsed = sscanf(line, " \"%d\": [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]", 
                                     &keyCode, &counts[0], &counts[1], &counts[2], &counts[3], 
                                     &counts[4], &counts[5], &counts[6], &counts[7], 
                                     &counts[8], &counts[9], &counts[10], &counts[11], 
                                     &counts[12], &counts[13], &counts[14], &counts[15]);
            if (itemsParsed == NUM_MODIFIER_COMBINATIONS + 1 && keyCode >= 0 && keyCode < MAX_KEY_CODE) { // Check if all items were successfully parsed
                for (int i = 0; i < NUM_MODIFIER_COMBINATIONS; i++) {
                    keyCounts[keyCode][i] = counts[i];
                }
            } else {
                logTo(stderr, "Error parsing line: %s\n", line);
                exit(1);
            }
        }
    }
}
void loadKeyCountsCsv(FILE *logfile) {
    char line[1024];
    // skip first line as it's only headers
    if(fgets(line, sizeof(line), logfile) == NULL) {
        return; 
    }
    int currentLine = 2;
    while (fgets(line, sizeof(line), logfile) != NULL) {
        int keyCode, counts[NUM_MODIFIER_COMBINATIONS];
        int itemsParsed = sscanf(line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", 
                                    &keyCode, &counts[0], &counts[1], &counts[2], &counts[3], 
                                    &counts[4], &counts[5], &counts[6], &counts[7], 
                                    &counts[8], &counts[9], &counts[10], &counts[11], 
                                    &counts[12], &counts[13], &counts[14], &counts[15]);
        if (itemsParsed == NUM_MODIFIER_COMBINATIONS + 1 && keyCode >= 0 && keyCode < MAX_KEY_CODE) { // Check if all items were successfully parsed
            for (int i = 0; i < NUM_MODIFIER_COMBINATIONS; i++) {
                keyCounts[keyCode][i] = counts[i];
            }
        } else {
            logTo(stderr, "Error parsing line %d (parsed %d of %d): %s\n", currentLine, itemsParsed, NUM_MODIFIER_COMBINATIONS+1, line);
            exit(1);
        }
        currentLine += 1;
    }
}


void loadKeyCounts() {
    FILE *logfile = fopen(logfileLocation, "r");
    if (!logfile) {
        logTo(stdout, "Log file does not exist. Starting fresh.");
        return; // File doesn't exist, proceed with the default initialization
    }
    if(useJson) {
        loadKeyCountsJson(logfile);
    }
    else {
        loadKeyCountsCsv(logfile);
    }
    fclose(logfile);
    logTo(stdout, "Loaded existing key counts from log file.");
}


CGEventRef CGEventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *refcon) {
    // Early exit for non-keydown events to minimize processing
    if (type != kCGEventKeyDown && type != kCGEventFlagsChanged) return event;

    // Extract keyCode and modifier flags
    CGKeyCode keyCode = (CGKeyCode)CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode);
    CGEventFlags flags = CGEventGetFlags(event);

    // Early exit if keyCode is out of bounds
    if (keyCode >= MAX_KEY_CODE) return event;
    
    bool down = false;
    if (type == kCGEventFlagsChanged) {
        switch (keyCode) {
        case 54: // [right-cmd]
        case 55: // [left-cmd]
            down = (flags & kCGEventFlagMaskCommand) && !(lastFlags & kCGEventFlagMaskCommand);
            break;
        case 56: // [left-shift]
        case 60: // [right-shift]
            down = (flags & kCGEventFlagMaskShift) && !(lastFlags & kCGEventFlagMaskShift);
            break;
        case 58: // [left-option]
        case 61: // [right-option]
            down = (flags & kCGEventFlagMaskAlternate) && !(lastFlags & kCGEventFlagMaskAlternate);
            break;
        case 59: // [left-ctrl]
        case 62: // [right-ctrl]
            down = (flags & kCGEventFlagMaskControl) && !(lastFlags & kCGEventFlagMaskControl);
            break;
        case 57: // [caps]
            down = (flags & kCGEventFlagMaskAlphaShift) && !(lastFlags & kCGEventFlagMaskAlphaShift);
            break;
        default:
            break;
        }
    } else if (type == kCGEventKeyDown) {
        down = true;
    }
    lastFlags = flags;

    // Only log key down events.
    if (!down) {
        return event;
    }

    // If the caps lock key is pressed, set the shift flag
    bool caps = flags & kCGEventFlagMaskAlphaShift;
    if (caps) {
        if (isKeyAffectedByCaps[keyCode]) {
            flags |= kCGEventFlagMaskShift;
        }
    }
    int modifierIndex = getModifierFlagIndex(flags);
    
    if (keyCode < MAX_KEY_CODE) {
        keyCounts[keyCode][modifierIndex]++;
        setAlarm();
    }

    return event;
}

void setupSignalHandlers() {
    signal(SIGALRM, timerHandler);
    signal(SIGINT, siginttermHandler);
    signal(SIGTERM, siginttermHandler);
}

void cleaunp() {
    saveKeyCounts();
}
int main(int argc, const char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            fprintf(stdout, "Usage: keycounter [--json | --csv] [logfile]");
            exit(0);
        }
    }
    logToNoNewLine(stdout, "Starting keycounter with arguments:");
    for (int i = 0; i < argc; i++) {
        fprintf(stdout, " \"%s\"", argv[i]);
    }
    fprintf(stdout, "\n");
    
    // Log the created time of the executable file
    struct stat attrib;
    if (stat(argv[0], &attrib) == 0) {
        char createdTime[80];
        strftime(createdTime, sizeof(createdTime), "%Y-%m-%dT%H:%M:%S%z", localtime(&attrib.st_ctime));
        logTo(stdout, "Executable file created time: %s", createdTime);
    } else {
        logTo(stderr, "ERROR: Unable to retrieve executable file information.");
    }

    const char *dirPath = argv[0];
    char *lastSlash = strrchr(dirPath, '/');
    if (lastSlash) {
        *lastSlash = '\0';  // Null-terminate the string at the last slash
    }

    // Create an event tap to retrieve keypresses.
    CGEventMask eventMask = CGEventMaskBit(kCGEventKeyDown) | CGEventMaskBit(kCGEventFlagsChanged);
    CFMachPortRef eventTap = CGEventTapCreate(
        kCGSessionEventTap, kCGHeadInsertEventTap, 0, eventMask, CGEventCallback, NULL
    );

    // Exit the program if unable to create the event tap.
    if (!eventTap) {
        logTo(stderr, "ERROR: Unable to create event tap. Terminating.\n");
        exit(1);
    }

    // Clear the logfile if clear argument used or log to specific file if given.
    // Set default values:
    useJson = false;
    logfileLocation = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            logTo(stdout, "Usage: keycounter [--json|--csv] [logfile]");
            exit(0);
        }
        else if (strcmp(argv[i], "--json") == 0) {
            useJson = true;
        } else if (strcmp(argv[i], "--csv") == 0) {
            useJson = false;
        } else if (strncmp(argv[i], "--", 2) != 0) {
            logfileLocation = argv[i];
        }
    }
    if (logfileLocation == NULL) {
        char filePath[1024];
        strcpy(filePath, dirPath);
        strcat(filePath, "/keycounts");
        strcat(filePath, useJson ? ".json" : ".csv");
        logfileLocation = filePath;
    }

    initializeIsKeyAffectedByCaps();
    setupSignalHandlers();

    // Display the location of the logfile
    logTo(stdout, "Using log file: \"%s\" %s", logfileLocation, useJson ? "JSON": "CSV");
    fflush(stdout);

    // Load key counts from existing log file
    loadKeyCounts();

    // Create a run loop source and add enable the event tap.
    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, eventTap, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);

    // Start the run loop to begin receiving CGEvents.
    CFRunLoopRun();

    // Clean up.
    CFRelease(eventTap);
    CFRelease(runLoopSource);

    alarm(0);
    if (alarmSetTime != 0) {
        saveKeyCounts();
        logTo(stdout, "Saved key counts");
    }
    logTo(stdout, "Exiting keycounter\n");

    return 0;
}