# CMPUT379 Assignment 2
Specifications available in assignment2_specification.pdf

The deliverable includes a design document (DESIGN.txt or DESIGN.md) which will have to address the following broad questions, outlining the specific solution strategy, and pointing to the implementation details necessary (e.g. by referencing particular functions in your code) and what are the implications of your design decisions.

DISCLAIMER: When a client first connects, if they are colliding with another client, nothing will show up on the screen
until they are no longer colliding. I did it this way as a sort of simulation of a genuinely random initialization, if we
later choose to not use a seed for the randomizer, then the server will loop until it finds a good location for each client.
To get around this, just move any already connected clients away from the default location.

Also, it seems that if the server is killed while there are clients still connected there is likely to be a socket leak.
Trying to run the server shortly afterward will result in a "Bind Error". This is despite verifed graceful termination of
the server when there are no clients connected.



How do you ensure that input response time (keystroke read, transmission of command, receiving at the server) is minimized?

There is a dedicated input thread for receiving input on the client side, and a dedicated client thread on the server side, as well
as an overlord thread which is purely restricted to doing game state calculations as opposed to handling I/O. With this design as well
as the usage of non-blocking sockets, the program is able to swiftly react to input.

How do you minimize the amount of traffic flowing back and forth between clients and server?

The server and client only communicate when the game state has changed. This is ensured by select.

How do you handle multiple commands being sent from a client between two successive game board updates? (see also notes)
The client only sends the most recent input when prompted, all prior inputs are discarded.

How do you ensure that the same (consistent) state is broadcast to all clients?
Every client thread reads from one game state array which is passed by pointer.
Client threads never write to the game state array, but instead send move requests to
an overlord thread that maintains the game state by processing these inputs and broadcasting
results to all clients.

How do you ensure the periodic broadcast updates happen as quickly as possible?
Updates are made speedy by having clients read 'simultaneously' from a game state array
once the overlord has finished calculations. Client threads essentially 'spin', waiting for
permission to read.

How do you deal with clients that fail without harming the game state?
Clients that fail are handled by the corresponding client thread on the server side.
The corresponding socket is closed and the player is set as null, and will be overwritten
upon connection of a new client.
Client threads do not write to the game state directly.

How do you handle the graceful termination of the server when it receives a signal? (Provide and implement a reasonable form of graceful termination.) The server gracefully terminates by looping through a list of sockets and properly shutting down and closing all of them.
  This practice prevents socket leaks. The server also loops through a list of all the client threads and kills them.

Write and run a function that gracefully terminates the server whenever a signal is received.
See sig_handler in server.c

How do you deal with clients that unilaterally break the connection to the server? (Assume the client can exit, e.g. crash, without advance warning.) Crashed clients are once again handled by the client specific threads which sheild the rest of the server program from suffering from
crashed client consequences.


How can you improve the user interface without hurting performance?
Ncurses has colors, which would have been nice.
Adding a gui could make setup easier but the data sending between the client and server should be minimised.
