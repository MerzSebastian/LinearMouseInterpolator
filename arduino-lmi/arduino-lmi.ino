#include "USBHost_t36.h"
#include "./coordinates.h"
#include "./button.h"

USBHost usb_host;
USBHub hub(usb_host);
USBHIDParser hid(usb_host);
MouseController mouse(usb_host);

#define TX_SIZE 5
#define RX_SIZE 3

int8_t tx[TX_SIZE];
int8_t rx[RX_SIZE];

bool debug = false;

int movesPerMinute = 450;
int moves[29][2] = {{0,0}, {-7, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40},{-20, 40}};
float movesPerMilliseconds = round(1000 / (movesPerMinute / 60));
bool mousePressed = false;
bool shouldReset = true;
bool shouldAnimate = true;
unsigned long startTime = millis();
unsigned long nextMove = millis();
unsigned short animationTimes = sizeof(moves) / sizeof(moves[0]);
unsigned short animationCounter = 0;
struct intCoordinates lastProgression = createIntCoordinates(0, 0);
struct intCoordinates pixelsToMove = createIntCoordinates(0, 0);
struct floatCoordinates overAllRest = createFloatCoordinates(0, 0);

bool buttonPressed(Button state) { return usb_mouse_buttons_state == static_cast<int>((int)state); }
void moveMouse(struct intCoordinates cords) { usb_mouse_move(mouse.getMouseX() + cords.x*1, mouse.getMouseY() + cords.y*1, mouse.getWheel(), mouse.getWheelH()); }
intCoordinates translateToCorrectDirection(intCoordinates move, intCoordinates originalMove) { return createIntCoordinates(originalMove.x > 0 ? move.x : move.x * -1, originalMove.y > 0 ? move.y : move.y * -1); }

void inputToHost()
{
  usb_mouse_buttons_state = mouse.getButtons();
  usb_mouse_move(mouse.getMouseX(), mouse.getMouseY(), mouse.getWheel(), mouse.getWheelH());
}

void log(String text) {
  if (debug) { Serial1.println(text); }
}

void setup()
{
  Serial1.begin(115200);
  usb_host.begin();
}

void loop()
{
  usb_host.Task();
  if (mouse.available())
  {
    inputToHost();
    mouse.mouseDataClear();
  }
  if (buttonPressed(Button::LeftRight))
  {
    if (shouldReset)
    {
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
    if (shouldAnimate)
    {
      float elapsedTimeRound = movesPerMilliseconds - (nextMove - millis());
      struct intCoordinates currentMove = createIntCoordinates(moves[animationCounter][0], moves[animationCounter][1]);
      if (nextMove <= millis())
      {
        if (animationCounter > 0)
        {
          struct intCoordinates adjustedMove = translateToCorrectDirection(createIntCoordinates((int)(abs(currentMove.x) - pixelsToMove.x), (int)(abs(currentMove.y) - pixelsToMove.y)), currentMove);
          log("moving => x, y: " + String(adjustedMove.x) + ", " + String(adjustedMove.y) + " <= move the rest of the way");
          moveMouse(adjustedMove);
          log("next Interpolation: " + String(animationCounter) + ", animationTimes: " + String(animationTimes) + ", nextMove x,y: " + String(currentMove.x) + "," + String(currentMove.y));
        }
        
        nextMove += movesPerMilliseconds;
        if (animationCounter == animationTimes - 1)
        {
          shouldAnimate = false;
          animationCounter = 0;
        }

        lastProgression = createIntCoordinates(0, 0);
        overAllRest = createFloatCoordinates(0, 0);
        animationCounter += 1;
      }
      else {
        //Calculate Interpolation
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
  }
  else
  {
    shouldReset = true;
  }
}
