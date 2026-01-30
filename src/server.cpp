#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <signal.h>
#include <errno.h>
#include <algorithm>

#include "common.h"
#include "git_version.h"

#define MAX_NUM_CONNECTIONS 5

static std::string socket_path;
static int socket_fd = -1;

/**
 * @brief Signal handler to clean up the socket file and exit the program.
 * 
 * @param signum The signal number received
 */
static void unlink_and_exit(int signum) {
    if (socket_fd >= 0) close(socket_fd);
    if (!socket_path.empty()) unlink(socket_path.c_str());
    std::cout << "Terminating..." << std::endl;
    exit(0);
}

/**
 * @brief Handle a command received from a client connection.
 * 
 * @param con The client connection file descriptor
 * 
 * @param cmd The command string received from the client
 */
static void handle_client_command(int con, const std::string &cmd) {
    std::string reply = "REJECTED";

    std::string cmd_upper = cmd;
    std::transform(cmd_upper.begin(), cmd_upper.end(), cmd_upper.begin(), [](unsigned char c){ return std::toupper(c); });
        
    if (cmd_upper == "PING") reply = "PONG";
    else if (cmd_upper == "VERSION") reply = GIT_COMMIT;

    send(con, reply.c_str(), reply.size(), 0);
}

/**
 * @brief Check if the specified socket path is available for use.
 * 
 * @param socket_path The path to the socket file
 * @return int 0 if the path is available, or an error code otherwise
 */
static int test_socket_path(const std::string &socket_path) {
    int err = 0;
    
    struct stat file_status;
    if (lstat(socket_path.c_str(), &file_status) != 0) {
        if (errno == ENOENT) {
            // File does not exist. OK to create socket.
            err = 0;
        } else {
            // Some other error.
            err = errno;
        }
        return err;
    }

    if (!S_ISSOCK(file_status.st_mode)) {
        // Not a socket
        return ENOTSOCK;
    }

    // Check if socket is in use
    int test_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path)-1);

    int connect_res = connect(test_fd, (struct sockaddr*)&addr, sizeof(addr));
    err = errno;
    close(test_fd);

    if (connect_res == 0) {
        return EADDRINUSE;
    }

    // Delete file if it is a stale socket
    if (err == ECONNREFUSED) {
        if (unlink(socket_path.c_str()) != 0) {
            err = errno; 
        } else {
            err = 0;
        }
    }

    return err;
}

/**
 * @brief Initialize a UNIX socket at the specified path.
 * 
 * This function checks if the specified socket path is available, creates a UNIX socket,
 * binds it to the path, and starts listening for incoming connections.
 * 
 * @param path The path where the socket will be created
 * 
 * @return int The file descriptor of the created socket, or -1 on error.
 */
static int initialize_socket(const std::string &path) {
    int err = test_socket_path(path);
    if (err != 0) {
        std::cerr << "Socket unavailable: " << strerror(err) << std::endl;
        errno = err;
        return -1;
    }

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cerr << "socket() failed: " << strerror(errno) << std::endl;
        return -1;
    }

    socket_path = path;
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path)-1);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "bind() failed on '" << socket_path << "': " << strerror(errno) << std::endl;
        close(fd);
        return -1;
    }

    if (listen(fd, MAX_NUM_CONNECTIONS) < 0) {
        std::cerr << "listen() failed on '" << socket_path << "': " << strerror(errno) << std::endl;
        close(fd);
        return -1;
    }

    return fd;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <unix-socket-path>\n";
        return 1;
    }
    
    signal(SIGINT, unlink_and_exit);
    signal(SIGTERM, unlink_and_exit);

    int err = 0;

    std::string socket_path = argv[1];
    socket_fd = initialize_socket(socket_path);
    if (socket_fd < 0) {
        std::cerr << "Failed to bind server socket" << std::endl;
        return errno;
    }

    std::cout << "Server listening on socket '" << socket_path << "'" << std::endl;

    while (true) {
        int con = accept(socket_fd, nullptr, nullptr);
        if (con < 0) {
            std::cerr << "accept() failed: " << strerror(errno) << std::endl;
            continue;
        }

        char buf[SERVER_REPLY_BUFFER_SIZE] = {0};
        ssize_t len = recv(con, buf, sizeof(buf)-1, 0);
        if (len <= 0) {
            std::cerr << "recv() failed: " << strerror(errno) << std::endl;
            close(con);
            continue;
        }

        std::string cmd = std::string(buf);
        std::cout << "Received command: \"" << cmd << "\"" << std::endl;

        handle_client_command(con, cmd);

        close(con);
    }

    unlink_and_exit(0);

    return 0;
}
