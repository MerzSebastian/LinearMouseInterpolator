#include "./button.h"
#include "./coordinates.h"
#include "USBHost_t36.h"
#include <SD.h>
#include <SPI.h>

#define TX_SIZE 5
#define RX_SIZE 3

USBHost usb_host;
USBHub hub(usb_host);
USBHIDParser hid(usb_host);
MouseController mouse(usb_host);
File file;

const int cs = BUILTIN_SDCARD;
const bool debug = true;

int8_t tx[TX_SIZE];
int8_t rx[RX_SIZE];

int moves[500][2];
float movesPerMilliseconds;
unsigned long startTime = millis();
unsigned long nextMove = millis();
unsigned short movesPerMinute;
unsigned short animationTimes;
unsigned short animationCounter;
bool reverseX;
bool reverseY;
bool mousePressed = false;
bool shouldReset = true;
bool shouldAnimate = true;
struct intCoordinates lastProgression = createIntCoordinates(0, 0);
struct intCoordinates pixelsToMove = createIntCoordinates(0, 0);
struct floatCoordinates overAllRest = createFloatCoordinates(0, 0);

bool buttonPressed(Button state) { return usb_mouse_buttons_state == static_cast<int>((int)state); }
void moveMouse(struct intCoordinates cords) { usb_mouse_move(mouse.getMouseX() + cords.x * (reverseX ? -1 : 1), mouse.getMouseY() + cords.y * (reverseY ? -1 : 1), mouse.getWheel(), mouse.getWheelH()); }
intCoordinates translateToCorrectDirection(intCoordinates move, intCoordinates originalMove) { return createIntCoordinates(originalMove.x > 0 ? move.x : move.x * -1, originalMove.y > 0 ? move.y : move.y * -1); }

void inputToHost() {
    usb_mouse_buttons_state = mouse.getButtons();
    usb_mouse_move(mouse.getMouseX(), mouse.getMouseY(), mouse.getWheel(), mouse.getWheelH());
}

void log(String text) {
    if (debug) {
        Serial1.println(text);
    }
}

void parseConfig(String conf) {
    unsigned short lastDelimiterIndex = 3;
    unsigned short delimiterCounter = 0;
    unsigned short lastKeyPairCommaIndex = 0;
    reverseX = String(conf[0]) == "1";
    reverseY = String(conf[2]) == "1";
    String temp = "";
    log("initial data: " + conf);
    log("reverseX: " + String(reverseX));
    log("reverseY: " + String(reverseY));
    for (unsigned short i = 4; i < conf.length(); i++) {
        if (String(conf[i]) == ",") {
            lastKeyPairCommaIndex = i;
        }
        if (String(conf[i]) == "|") {
            if (temp != "") {
                moves[delimiterCounter][0] = conf.substring(lastDelimiterIndex + 1, lastKeyPairCommaIndex).toInt();
                moves[delimiterCounter][1] = conf.substring(lastKeyPairCommaIndex + 1, i).toInt();
                log("firstVal: " + String(moves[delimiterCounter][0]) + " - secondVal: " + String(moves[delimiterCounter][1]));
            }
            temp = "";
            delimiterCounter += 1;
            lastDelimiterIndex = i;
        } else {
            temp += conf[i];
        }
    }
    movesPerMinute = conf.substring(lastDelimiterIndex + 1, conf.length()).toInt();
    movesPerMilliseconds = round(1000 / (movesPerMinute / 60));
    animationTimes = delimiterCounter;
    log("movesPerMinute: " + String(movesPerMinute));
    log("moveCount: " + String(animationTimes));
}

void setup() {
    Serial1.begin(115200);
    usb_host.begin();
    while (!Serial1) {
        ;
    }

    if (!SD.begin(cs)) {
        log("sd card initialization failed!");
        return;
    }
    log("sd card initialization done.");
    file = SD.open("config.txt");
    if (file) {
        while (file.available()) {
            parseConfig(file.readString());
        }
        file.close();
    } else {
        log("error opening config.txt");
    }
}

void loop() {
    usb_host.Task();
    if (mouse.available()) {
        inputToHost();
        mouse.mouseDataClear();
    }
    if (buttonPressed(Button::Right)) {
        if (shouldReset) {
            log("# next animation: " + animationCounter);
            startTime = millis();
            nextMove = millis();
            animationCounter = 0;
            shouldReset = false;
            shouldAnimate = true;
            lastProgression = createIntCoordinates(0, 0);
            pixelsToMove = createIntCoordinates(0, 0);
            overAllRest = createFloatCoordinates(0, 0);
        }
        if (shouldAnimate) {
            float elapsedTimeRound = movesPerMilliseconds - (nextMove - millis());
            struct intCoordinates currentMove = createIntCoordinates(moves[animationCounter][0], moves[animationCounter][1]);
            if (nextMove <= millis()) {
                if (animationCounter > 0) {
                    struct intCoordinates adjustedMove = translateToCorrectDirection(createIntCoordinates((int)(abs(currentMove.x) - pixelsToMove.x), (int)(abs(currentMove.y) - pixelsToMove.y)), currentMove);
                    log("moving => x, y: " + String(adjustedMove.x) + ", " + String(adjustedMove.y) + " <= move the rest of the way");
                    moveMouse(adjustedMove);
                    log("next Interpolation: " + String(animationCounter) + ", animationTimes: " + String(animationTimes) + ", nextMove x,y: " + String(currentMove.x) + "," + String(currentMove.y));
                }

                nextMove += movesPerMilliseconds;
                if (animationCounter == animationTimes - 1) {
                    shouldAnimate = false;
                    animationCounter = 0;
                }

                lastProgression = createIntCoordinates(0, 0);
                overAllRest = createFloatCoordinates(0, 0);
                animationCounter += 1;
            } else {
                // Calculate Interpolation
                struct floatCoordinates moveEveryXms = createFloatCoordinates((float)movesPerMilliseconds / (float)abs(currentMove.x), (float)movesPerMilliseconds / (float)abs(currentMove.y));
                struct floatCoordinates movePixels = createFloatCoordinates((float)elapsedTimeRound / moveEveryXms.x, (float)elapsedTimeRound / moveEveryXms.y);
                struct floatCoordinates rest = createFloatCoordinates(movePixels.x - (int)movePixels.x, movePixels.y - (int)movePixels.y);
                overAllRest = createFloatCoordinates(overAllRest.x + rest.x, overAllRest.y + rest.y);
                pixelsToMove = createIntCoordinates((int)(movePixels.x - rest.x + (int)overAllRest.x), (int)(movePixels.y - rest.y + (int)overAllRest.y));

                struct intCoordinates adjustedMove = translateToCorrectDirection(createIntCoordinates(pixelsToMove.x - lastProgression.x, pixelsToMove.y - lastProgression.y), currentMove);
                log("moving => x, y: " + String(adjustedMove.x) + ", " + String(adjustedMove.y));
                moveMouse(adjustedMove);

                lastProgression = pixelsToMove;
                overAllRest = createFloatCoordinates(overAllRest.x - (int)overAllRest.x, overAllRest.y - (int)overAllRest.y);
            }
        }
    } else {
        shouldReset = true;
    }
}
