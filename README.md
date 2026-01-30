# socket-comm

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Usage

SYNOPSIS

    server [OPTIONS] <socket-path>
    client [OPTIONS] <socket-path> <COMMAND>

SERVER

    server <socket-path>

    Description
        Listen on the socket at <socket-path>.
        The socket is created if it does not already exist.
        The server replies to client commands

    Supported commands
        VERSION
            Reply with the git commit hash of the build.

        PING
            Reply with "PONG" (just a bit of fun 🙂)

        Any other command
            Reply with `REJECTED`.

CLIENT

    client <socket-path> <COMMAND>

    Description
        Connect to the socket at <socket-path>
        Send <COMMAND> to the server and print the reply to stdout.


EXIT STATUS

    0 on success; non-zero on error.
    Errors are printed to stderr.


## Docker

Build the Docker image (builds natively, tests, and cross-compiles for aarch64):

```bash
sudo docker build -t socket-comm .
```

Run the container (starts server, runs test commands, server stays running):

```bash
sudo docker run -it socket-comm
```

The container will:
1. Build the application natively
2. Run test commands (VERSION, PING, UNKNOWN)
3. Download and extract the Bootlin aarch64 toolchain
4. Cross-compile for aarch64
5. Start the server listening on `/tmp/socket`
6. Keep running until you press `Ctrl+C` or run `docker stop <container-id>`

To stop the container gracefully:

```bash
sudo docker stop <container-id>
```
