# FW Client

## Install

Extract the archive to a location of your choice.

If it doesn't run download the Visual C++ Redistributable for Visual Studio 2015
package (https://www.microsoft.com/en-us/download/details.aspx?id=48145) from 
Microsoft and install it.

## Uninstall

Delete the extracted files.

## Run

1. Start `fw.exe`
2. Create an account
3. Create a character
4. Enter world

## Input

### Keys

Key bindings are not customizable yet.

* `W`, `Up`: Move forward
* `A`, `Left`: Turn left
* `S`, `Down`: Move backward
* `D`, `Right`: Turn right
* `Q`: Move left
* `E`: Move right
* `R`: Keep running
* `Space`: Goto selected object or follow selected object when it's moving
* `Ctrl`: Highlight objects

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
