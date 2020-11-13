# Assignment 6 

### Initialization

Warning: Using gcc version other than 9.3.0 may cause problems. Program does not gurantee cross compatibility.

Starting the server:

1. Compile the server using `make server`
2. Run the server as: `./server <PORT>`

Starting a client:

1. Compile the client using `make client`
2. Run the client as: `./client <IPv4-ADDRESS> <SERVER-PORT>`

### Setting the socket

Socket serves are abstracted into an integer just like file descriptors. We set the socket based on the protocol and the format of packet transmission used using a structure.

ipv4 (AF_INET) is used along with SOCKET_STREAM as the format for transmission.

We create a structure that represents the details of the connection such as the port number. We also let the system decide what address to use for communication.

After this we bind the chosen socket (from the socket descriptor) to the given details.

After the socket has been bound we make the socket go passive. This will allow it to accept the incoming requests. 

Using the socket we registered, we wait for the client, the program is blocked until a client tries to connect.

## Client side

The initialization just involves the setting up of the socket and trying to connect to the server. The socket connection is again done using the structure.

# Main protocol

### Server

The server after initialization waits for the client connection, once there is a connection it goes into client loop.

The client loop is where the server waits to read from the common socket buffer for inputs.

Once received it starts sending the file and after it's done, it waits for more commands from the server-side.

### Client

The client takes input from the standard input, checks the command, and based on the command the data is transferred to the server. 

Commands:

1. `ls` : Lists files in the current directory in the server
2. `cd` : Change server directory
3. `get <ARGLIST>` : Takes a ARGLIST of files and attempts to download them from server's current directory

To exit as the client, press `<C-D>`
