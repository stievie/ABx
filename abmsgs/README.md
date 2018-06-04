# abmsgs

Message server, mostly for relaying chat messages, but also responsible to notify
players of friends status.

Only game server are connected to this server (not the clients).

Only one running instance of this server is allowed, and all game server connect
to this running instance.

## If a player chats with a player on another server:

1. it pushes the message to this server,
2. this server identifies the game server to which the target is connected
3. sends this game server a message
4. The game server sends it to the player.

## If a players status changes to 'Online':

1. Gets friendlist, guild list
2. notifies all game servers with interested parties
