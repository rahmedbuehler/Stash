#include <cstdio> // perror
#include <iostream>
#include <stdlib.h> // perror
#include <unistd.h> //gethostname
#include <cerrno> //errno
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h> //getpeername
#include <arpa/inet.h> //inet_pton

// Returns pointer to the <sin_addr> or <sin6_addr> in <sa>
void * get_in_addr(struct sockaddr * sa)
{
        if (sa->sa_family == AF_INET)
        {
            return &(((struct sockaddr_in *)sa)->sin_addr);
        }
        return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Returns socket file descriptor for the Stash server
int connect_to_server()
{
    struct addrinfo hints {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Temporary while current host is server
    char server[256];
    gethostname(server, sizeof(server));
    const char* port{"3490"};

    // Get acceptable addrinfo structs
    struct addrinfo * gai_result;
    int gai_return_value;
    gai_return_value = getaddrinfo(server, port, &hints, &gai_result);
    if (gai_return_value != 0)
    {
        throw gai_strerror(gai_return_value);
    }

    // Loop through all the results and connect to the first we can
    int sockfd;
    struct addrinfo * current_ai;
    for(current_ai = gai_result; current_ai != nullptr; current_ai = current_ai->ai_next)
    {
        sockfd = socket(current_ai->ai_family, current_ai->ai_socktype, current_ai->ai_protocol);
        if (sockfd == -1)
        {
            std::perror("stash_client - socket operation");
            continue;
        }
        if (connect(sockfd, current_ai->ai_addr, current_ai->ai_addrlen) == -1)
        {
            std::perror("stash_client - connect operation");
            close(sockfd);
            continue;
        }
        break;
    }

    // No connection found
    if (current_ai == nullptr)
    {
        throw "stash_client failed to connect";
    }

    // Output
    char ip_string[INET6_ADDRSTRLEN];
    inet_ntop(current_ai->ai_family, get_in_addr(static_cast<struct sockaddr *>(current_ai->ai_addr)), ip_string, sizeof ip_string);
    std::cout << "stash_client: connecting to " << ip_string << "\n";

    freeaddrinfo(gai_result);
    return sockfd;
}

int main(int argc, char *argv[])
{
    try
    {
        int server_sockfd {connect_to_server()};
        int maxdatasize {100};
        char buf[maxdatasize];
        long int numbytes {recv(server_sockfd, buf, maxdatasize-1, 0)};
        if (numbytes == -1)
        {
            std::perror("stash_client - recv operation");
            throw "stash_client recv failed";
        }
        buf[numbytes] = '\0';
        std::cout << "stash_client: received " << numbytes << " bytes\n";
        std::cout << "stash_client: received '" << buf <<"'\n";

        close(server_sockfd);
    }
    catch (...)
    {
        std::cerr << "stash_client: an unexpected exception occurred.\n";
    }
    return 0;
}

