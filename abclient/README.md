# FW Client

This is the game client you should use to play this game.

## Install

Extract the archive to a location of your choice.

If it doesn't run download the Visual C++ Redistributable for Visual Studio 2019
package (https://visualstudio.microsoft.com/downloads/ -> Other Tools and Frameworks)
from Microsoft and install it.

## Uninstall

1. Delete the extracted files.
2. Delete the directory `c:\Users\<username>\AppData\Roaming\Trill\FW\` (Win7)

## Run

1. Start `fw.exe`
2. Create an account (you can/should leave the Email field empty)
3. Create a character
4. Enter world

You can run multiple instances of the client at the same time, however you can't
login twice with the same account. If you want that multiple instances use different
settings, use the `-prefpath` command line switch.

## Input

### Keys

Key bindings are fully customizeable with the Options -> Input window. The default
bindings are:

* `W`, `Up`: Move forward
* `A`, `Left`: Turn left
* `S`, `Down`: Move backward
* `D`, `Right`: Turn right
* `Q`: Move left
* `E`: Move right
* `R`: Keep running
* `Space`: Default action, attack or follow
* `Y`: Reverse camera
* `F`: Select self
* `T`: Select called target
* `LeftCtrl`: Highlight objects
* `PrtScr`: Take screenshot
* `^`: Show/hide chat window
* `Backspace`: Hide UI
* `Pause`: Mute audio

Other shortcuts are shown in `[` `]`.

### Mouse

* LMB: Select object, goto point
* RMB: Mouse look
* Mouse wheel: Zoom

## Chat commands

~~~
/a <message>: General chat
/g <message>: Guild chat
/party <message>: Party chat
/w <name>, <message>: Whisper to <name> a <message>
/roll <number>: Rolls a <number>-sided die (2-100 sides)
/resign: Resign
/age: Show Character age
/hp: Show health points and energy
/stuck: Force server position
/ip: Show server IP
/help: Show this help
~~~

## Command line arguments

* `-prefpath <directory>`: Preferecences path
* `-username <username>`: Username
* `-password <password>`: Password
* `-no-cp`: No client prediction

## Settings

It saves the settings on exit to the directory `c:\Users\<username>\AppData\Roaming\Trill\FW\` (Win7)
or if the `-prefpath` command line argument is given, to this directory.

## Config file

Basic configuration is read from the configuration file `config.xml` in the program
directory.

### Required entries

LoginHost and LoginPort tells the game client where to connect to.
~~~xml
  <parameter name="LoginHost" type="string" value="localhost" />
  <parameter name="LoginPort" type="int" value="2748" />
~~~

### Optional entries

If you don't want to enter your username and password every time you login you
can create a `Username` and `Password` entry:
~~~xml
  <parameter name="Username" type="string" value="the_username"/>
  <parameter name="Password" type="string" value="the_password"/>
~~~

If you run different instances of the server (e.g. Test and Production) you can
create so called Environments:
~~~xml
  <parameter name="Environment">
    <name type="string" value="Production" />
    <host type="string" value="192.168.1.135" />
    <port type="int" value="2748" />
  </parameter>
  <parameter name="Environment">
    <name type="string" value="Test" />
    <host type="string" value="192.168.1.113" />
    <port type="int" value="2748" />
  </parameter>
  <parameter name="Environment">
    <name type="string" value="Development" />
    <host type="string" value="localhost" />
    <port type="int" value="2748" />
  </parameter>
~~~
This will create a dropdown list on the Login screen, which lets you chose the
server to connect to.

If you have such Environments, the `LogginHost` and `LoginPort` entries mentioned
above, are ignored.
