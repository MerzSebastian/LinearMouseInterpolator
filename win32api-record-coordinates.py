import win32api
import time

state_left = win32api.GetKeyState(0x01)
state_right = win32api.GetKeyState(0x02)
results = []
print("Start recording!")
print("Left-click for new record, right-click to stop recording.")
while True:
    new_state_left = win32api.GetKeyState(0x01)
    new_state_right = win32api.GetKeyState(0x02)

    if new_state_left != state_left:
        state_left = new_state_left
        if new_state_left < 0:
            tx, ty = win32api.GetCursorPos()
            results.append([tx, ty])
            print("Added: " + str([tx, ty]))

    if new_state_right != state_right:
        state_right = new_state_right
        if new_state_right < 0:
            print("Stop recording!")
            break;
    time.sleep(0.001)
    
#print("recorded positions: " + str(results))

realtiveResults = []
for i in range(1, len(results)):
    realtiveResults.append([results[i-1][0]-results[i][0], results[i-1][1]-results[i][1]])
    
#print("relative positions: " + str(realtiveResults))
#print("Result: {{0, 0}, " + str(realtiveResults).replace("[", "{").replace("]", "}")[1:])
sdCardResult = str(realtiveResults).replace("[", "|").replace("]", "|").replace(", |", "").replace(", ", ",")
#print()
movesPerMinute = input('Please enter movesPerMinute value:')
reverseX = input('Reverse x axis (y/n)?:')
reverseY = input('Reverse y axis (y/n)?:')
print("Result: " + str(0 if reverseX == "y" else 1) + "|" + str(0 if reverseY == "y" else 1) + "|0,0" + sdCardResult[1:len(sdCardResult)-1] + movesPerMinute)