# FW Client

## Install

Extract the archive to a location of your choice.

If it doesn't run download the Visual C++ Redistributable for Visual Studio 2015
package (https://www.microsoft.com/en-us/download/details.aspx?id=48145) from 
Microsoft and install it.

## Uninstall

1. Delete the extracted files.
2. Delete the directory `c:\Users\<username>\AppData\Roaming\Trill\FW\` (Win7)

## Run

1. Start `fw.exe`
2. Create an account (you can/should leave the Email field empty)
3. Create a character
4. Enter world

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
* `Y`: Reverse camera
* `F`: Select self
* `Space`: Goto selected object or follow selected object when it's moving
* `LeftCtrl`: Highlight objects
* `PrtScr`: Take screenshot
* `^`: Show/hide chat window
* `Backspace`: Hide UI

Other shortcuts are shown in `[` `]`.

### Mouse

* LMB: Select object, goto point
* RMB: Mouse look
* Mouse wheel: Zoom

## Chat commands

~~~
/a <message>: General chat
/w <name>, <message>: Whisper to <name> a <message>
/mail <name>, [<subject>:] <message>: Send mail to <name> with <message>
/inbox: Show your mail inbox
/read <index>: Read mail with <index>
/delete <index>: Delete mail with <index>
/roll <number>: Rolls a <number>-sided die (2-100 sides)
/age: Show Character age
/ip: Show server IP
/help: Show this help
~~~

## Command line arguments

* `-prefpath <directory>`: Preferecences path
* `-username <username>`: Username
* `-password <password>`: Password

## Settings

It saves the settings on exit to the directory `c:\Users\<username>\AppData\Roaming\Trill\FW\` (Win7)
or if the `-prefpath` command line argument is given, to this directory.
