# TCP File Transfer

A compact C project demonstrating a TCP client/server file-transfer protocol over IPv4.

## Features

- Transfers binary file contents over a TCP stream.
- Handles partial socket reads and writes.
- Serves only regular files in the server's `files/` directory, using simple filenames.
- Reports unavailable files with an explicit response status.

## Requirements

- A POSIX-compatible operating system
- A C compiler and `make`

## Build

From the repository root:

```bash
make
```

## Run locally

Start the server from its directory so its file root resolves correctly:

```bash
cd Server
../build/tcp_server
```

In another terminal, run the client from the repository root:

```bash
./build/tcp_client
```

Enter `server.txt` to display the sample file from `Server/files/`. The server listens on port `8080`; the client connects to `127.0.0.1`.

## Protocol

The client sends a NUL-terminated filename. The server responds with one status byte (`D` for data or `E` for error), followed by the file contents or an error message. The server closes the connection when the response is complete.

This is an educational example, not a production file-transfer service. It has no authentication or encryption and handles one client at a time; keep it on a trusted local network.

## Layout

```text
Client/TCP_Client.c      TCP client
Server/TCP_Server.c      TCP server
Server/files/            Files available to clients
Makefile                 Build and clean targets
```
