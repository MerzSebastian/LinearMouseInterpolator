# LinearMouseInterpolator
 
Nothing here yet. 

Little project to interpolate between coordinates (time based). Designed for Teensy 4.2 to pass trough mouse. Can replay mouse positions in smooth motion. 


Record coordinates for replay:
```
python3 .\win32api-record-coordinates.py
```
Info: You can also use the included batch file:

Sample result:
```
Start recording! Left-click for new record, right-click to stop recording.
Added: [1087, 747]
Added: [944, 943]
Added: [1243, 1014]
Added: [1092, 727]
Stop recording!
Please enter movesPerMinute value:100
Reverse x axis (y/n)?:n
Reverse y axis (y/n)?:n
Which directorie should be saved (empty for current folder)?:
Saving result to: config.txt
```
