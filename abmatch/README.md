# abmatch

Match making server. From a technical point of view, this is not a server but a 
client which connects to the message server.

So the idea is, the game server sends a message to the message server that a player
wants to get a game. The message server informs the match making server about that
and add the player to a queue. If there are enough players for a game, the match 
making server sends a message to the message server and this informs the game server
it should start a new game and map all the player to the game.

~~~plain
          /-queue->-\         /--msg-->-\          /---add player-->-\
+--------+           +--------+          +--------+                   +---------+
| Client |           | abserv |          | abmsgs |                   | abmatch |
+--------+           +--------+          +--------+                   +---------+
          \-<--map--/          \<--msg--/          \<-enough players-/
~~~
