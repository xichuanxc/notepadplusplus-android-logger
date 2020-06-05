# notepadplusplus-android-logger
A plugin for Notepad++ to se Android Log

From https://sourceforge.net/projects/androidlogger/

Thanks to the author, but the plugin has not been updated since 2015-05-18 version V1.2.7



REVISION NOTES FROM readme.txt in v1.2.7 source code:

[NOTE] 
Compatible with Notepad++ 6.5 later version 

[Install] 
1. Push AndroidLogger.dll under "plugins\" directory of Notepad++ 
2. Push AndroidLogger.xml under "plugins\Config\" directory of Notepad++ 

[Features] 
1. Support lexer fot App & Radio Log, and cutomizable 
2. Support capture log from device!
3. Support shell cmd on device.  
->The shell cmd line must start with '>' and at the top of doc. 
->The line with '#' is comment. 
->Empty line is permitted. 
->Freely use logcat, top & grep, tcpdump.
4. Support capture device screenshot, now just save at d:\device.bmp 
5. monkey support! (not stablity!)

[History]
I plan to decode the APK xml file next release!

V1.2.7 2015.5.17
1) Add interactive shell mode
2) Open the source

V1.2.6 2015.3.14
1) Add Filer support, you can pull or push your file. (Not support directory yet.)
2) Source code refactor, and will open source soon!

V1.2.5 2015.3.6
1) Fix sub-eidtor of notepad++ not shown any log, now support both editor
2) Add filer support for Android Device
3) Add monkey runner cmd support

For example:
WAIT|{'seconds':5,}
PRESS|{'name':'HOME','type':'downAndUp',}
WAIT|{'seconds':2,}
TOUCH|{'x':450,'y':1200,'type':'downAndUp',}
WAIT|{'seconds':0.5,}
TOUCH|{'x':350,'y':1200,'type':'downAndUp',}
WAIT|{'seconds':2,}
TYPE|{'message':'123456',}
PRESS|{'name':'MENU','type':'downAndUp',}


V1.2.3 2015.2.5
1) Compile option /MD -> /MT
2) Flush log buffer if timeout 20ms

V1.2.2 2015.1.30
1) Fix shell logcat exited itself! 
2) Add check state for toolbar & menu! 
3) Compatible with some devices, let shell cmd could done and exit normaly! 
4) After device screen captured, now displays the screenshot atomatically !


V1.2.1 
1) Fix lexer to compatible with leading or trailing spaces! 
2) Shell cmd now can exit its-self, not need click button! 

V1.2.0 
1) Support custom keyword, your keywords your color! 
2) auto lexer for shell logcat cmd 
3) optimization adb shell output efficiencily! 

V1.1.2 
1) Fix when open style dialog, the fold margin is shown, should hide it! 


V1.1.1 
1) Fix rgb issue, now device screenshot is ok! 
2) Log & Shell concurrent optimization! 

V1.1.0 
1) adb cmd will not timeout! 
2) thread concurrent optimization 
