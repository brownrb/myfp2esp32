myFP2ESP32
MANAGEMENT SERVER ADMIN PAGES
This file maintained by Sublime


ADMIN 1 SERVERS
ASCOM Alpaca Server
	State 			[Enable | Disable]
	Status 			[Start | Stop]
	Port 			[Value] [Set]

TCPIP Server
	State 			[Enable | Disable]
	Status 			[Start | Stop]
	Port 			[Value] [Set]

Web Server
	State 			[Enable | Disable]
	Status 			[Start | Stop]
	Port 			[Value] [Set]


ADMIN 2 OTA-DUCKDNS
DuckDNS
	Status 			[Running | Stopped]
	Domain			[String] [Set]
	Token			[String] [Set]
	Refresh Time  	[Value] [Set]
	(60-3500 seconds)
OTA
	Status 			[Running | Stopped]
	Name			[String] [Set]
	Password		[String] [Set]
	ID				[String] [Set]


ADMIN 3 MOTOR OPTIONS
Coil Power
	State 			[Enable | Disable]
Delay After Move
	State 			[Enable | Disable]
	Delay			[Value] [Set]
	(10-250 milli-seconds)
Motor Speed Delay
	Delay			[Value] [Set]
	(1000-14000 micro-seconds)
Park
	State 			[Enable | Disable]
	Status 			[Running | Stopped]
	Time			[Value] [Set]	
	(0 - 600s)
Reverse Direction
	State 			[Enable | Disable]
Step Size
	State 			[Enable | Disable]
	Value			[Value] [Set]
	(1-100)


ADMIN 4 BACKLASH
Backlash In
	State 			[Enable | Disable]
	Steps			[Value] [Set]
	(0-255)
Backlash Out
	State 			[Enable | Disable]
	Steps			[Value] [Set]
	(0-255)


ADMIN 5 HPSW
Home Position Switch
	State 			[Enable | Disable]
  	Messages		[Enable | Disable]
TMC2209 STALL GUARD SOURCE
					Stall_guard	| Physical_switch | None
TMC2209 Current		[Value]	[Set]
TMC2225 Current		[Value] [Set]


ADMIN 6 LEDS-JOYSTICKS-PUSH BUTTONS
IN-OUT LEDs
	State			[Enable | Disable]
	Mode			[Move | Pulse]
Push Buttons
	State			[Enable | Disable]
	Steps 			[Value] [Set]			
	(0-StepSize/2)
Joysticks
	1 State			[Enable | Disable]
	2 State			[Enable | Disable]


ADMIN 7 DISPLAY
Display
	State			[Enable | Disable]
	Status			[Start | Stop]
	Show Pos When Moving [Enabled | Disabled]
	Page Time 		[Value] [Set]
	(2-10 seconds)
	Page Option 	[Checkboxes] [Set]


ADMIN 8 TEMPERATURE
Temperature Probe
	State 			[Enable | Disable]
 	Mode 			[C | F]
	Resolution		[9=0.5 - 12=0.0125]
	TC Available	[Enabled | Disabled]
	TC Direction	[In | Out]
	TC State		[Enabled | Disabled]
	TC Coefficient 	[0-400] [Set]


ADMIN 9 MISC
Controller Mode  	[ACCESSPOINT | STATIONMODE]
Static IP			[On | Off]
DEVICE NAME			[Text] [Set]
File List Format	[Long | Short]
Web Page Colors
	Title			[6-hex-digits-for RGB]
	Background		[6-hex-digits-for RGB]
	Header			[6-hex-digits-for RGB]
	Text			[6-hex-digits-for RGB]
