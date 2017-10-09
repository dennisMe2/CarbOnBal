# CarbOnBal
Carburetor and Throttle-body balancer / synchronizer software with D.I.Y. hardware.

This is an Arduino based project for balancing carbs and throttle body syncing.
Its relatively easy to build the hardware, it was designed with DIY in mind. 
Perhaps even as a first Arduino plus hardware project if you have the skills.

The software should be easy enough using the Arduino IDE (or Platformio).
The hardware consists mainly of an Arduino (or clone), HD54470 Display with 4 lines of 20 characters. 
4 cheap MAP sensors (in my case) and four buttons (OK, < >, CANCEL) to navigate the menu system.
The display displays bar graphs that indicate how well the throttle bodies or carbs are in sync.
This makes it easy to tweak them for that super smooth ride we all love.

The hardware I built makes use of the following library, which you can simply install in your Arduino Libs directory.
https://github.com/omersiar/ShiftedLCD

Note that it is possible to build this basic version with a straight "four wire" interface in stead of the SPI shifted version I used. This might be easyer but it will prevent the connection of extra features in future. I'm looking into making a simpler version using this, but I want to tweak the prototype and make it insanely user friendly first.

I'm documenting the project on this Hackaday site:
https://hackaday.io/project/27569-carbonbal
If you're interested in building this as a D.I.Y.project then that's the place to go. It will eventually contain a step-by-step guide on building this.
