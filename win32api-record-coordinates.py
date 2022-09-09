state_left = win32api.GetKeyState(0x01)
state_right = win32api.GetKeyState(0x02)
results = []
print("start recording!")
while True:
    new_state_left = win32api.GetKeyState(0x01)
    new_state_right = win32api.GetKeyState(0x02)

    if new_state_left != state_left:
        state_left = new_state_left
        if new_state_left < 0:
            tx, ty = win32api.GetCursorPos()
            results.append([tx, ty])
            print("added: " + str([tx, ty]))

    if new_state_right != state_right:
        state_right = new_state_right
        if new_state_right < 0:
            print("stop recording!")
            break;
    time.sleep(0.001)
    
print("recorded positions: " + str(results))

realtiveResults = []
for i in range(1, len(results)):
    realtiveResults.append([results[i-1][0]-results[i][0], results[i-1][1]-results[i][1]])
    
print("relative positions: " + str(realtiveResults))