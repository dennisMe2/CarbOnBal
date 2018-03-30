# CarbOnBal
Carburetor and Throttle-body balancer / synchronizer software with D.I.Y. hardware.

This is an Arduino based project for balancing carbs and throttle body syncing.
Its relatively easy to build the hardware, it was designed with DIY in mind. 
I'm sure its suited as a first Arduino plus hardware project if you have some basic skills.

The software should be easy enough to upload using the Arduino IDE (or of course your favorite IDE if you have a favorite).
Please note that, should you choose to download a .zip file, you will need to rename the unzipped folder to "CarbOnBal". That is, strip off any extensions such as a release number or "-master", otherwise the Arduino IDE won't recognize it. Thanks TheRedOne51 for the tip!

The hardware consists mainly of an Arduino (or clone), HD54470 Display with 4 lines of 20 characters. 
4 cheap MAP sensors (in my case) and four buttons (OK, < >, CANCEL) to navigate the menu system.
The display displays bar graphs that indicate how well the throttle bodies or carbs are in sync.
This makes it easy to tweak them for that super smooth ride we all love.

The hardware I built makes use of either the standard LiquidCrystal library or the New-LiquidCrystal bt Francesco Malpartida.
https://bitbucket.org/fmalpartida/new-liquidcrystal/overview
I recommend this newer library because its fater and lighter, but at this point in time ther appears to be no improvement in real-world use. I'll update this andmy recommendations if that changes.

The CarbOnBal Basic version is built with a straight "four wire" interface instead of the SPI shifted version I used for the first prototype. I also tested an I2C version in the mean time but that was much slower, required more memory than my Arduino had available at the time and did not allow for proper PWM backlight and contrast control. Both of there are a matter of user friendliness I didn't want to sacrifice. Besides there are a number of different I2C interfaces out there which all required tweaking hardware settings at an expert level. In short I discarded the I2C approach in favor of soldering a few more wires but less piddling with settings and more free SRAM (memory).

I'm documenting the project on this Hackaday site:
https://hackaday.io/project/27569-carbonbal
If you're interested in building this as a D.I.Y.project then that's the place to go. It will eventually contain a complete step-by-step guide on building this. For now it should contain enough of a project description to get you started!
