# LinearMouseInterpolator
 
Nothing here yet. 

Little project to interpolate between coordinates (time based). Designed for Teensy 4.2 to pass trough mouse. Can replay mouse positions in smooth motion. 


Record coordinates for replay:
```
python3 .\win32api-record-coordinates.py
```
Sample result:
```
PS C:\Users\*****\Documents\GitHub\LinearMouseInterpolator> python3 .\win32api-record-coordinates.py
Left-click for new record, right-click to stop recording.
Added: [2349, 343]
Added: [2178, 406]
Added: [2117, 545]
Added: [2330, 645]
Stop recording!
Result: {{0, 0}, {171, -63}, {61, -139}, {-213, -100}}
```