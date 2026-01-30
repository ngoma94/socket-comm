#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <errno.h>

#include "common.h"


int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <unix-socket-path> <COMMAND>\n";
        return EINVAL;
    }

    std::string socket_path = argv[1];
    std::string cmd = argv[2];

    int err = 0;

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        err = errno;
        std::cerr << "socket() failed: " << strerror(err) << std::endl;
        return err;
    }

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path)-1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        err = errno;
        std::cerr << "connect() to '" << socket_path << "' failed : " << strerror(err) << std::endl;
        close(fd);
        return err;
    }

    if (send(fd, cmd.c_str(), cmd.size(), 0) < 0) {
        err = errno;
        std::cerr << "send() failed: " << strerror(err) << std::endl;
        close(fd);
        return err;
    }

    char buf[SERVER_REPLY_BUFFER_SIZE] = {0};
    ssize_t len = recv(fd, buf, sizeof(buf)-1, 0);
    if (len < 0) {
        err = errno;
        std::cerr << "recv() failed: " << strerror(err) << std::endl;
        close(fd);
        return err;
    }

    std::string reply = "Response: \"" + std::string(buf) + "\"\n";
    std::cout << reply << std::endl;

    close(fd);
    return 0;
}
