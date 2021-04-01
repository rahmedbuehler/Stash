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
#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void* get_in_addr(struct sockaddr * sa)
{
        if (sa->sa_family == AF_INET) {
                return &(((struct sockaddr_in*)sa)->sin_addr);
        }

        return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    struct addrinfo hints {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Get acceptable addrinfo structs
    char host[256];
    gethostname(host, sizeof(host));
    struct addrinfo * gai_result;
    int gai_return_value;
    gai_return_value = getaddrinfo(host, PORT, &hints, &gai_result);
    if (gai_return_value != 0)
    {
        std::cerr << "getaddrinfo Error: " << gai_strerror(gai_return_value) << "\n";
        return 1;
    }

    // Loop through all the results and connect to the first we can
    int sockfd;
    struct addrinfo * current_ai;
    for(current_ai = gai_result; current_ai != NULL; current_ai = current_ai->ai_next)
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
    if (current_ai == NULL)
    {
        std::cerr << "Error: Failed to connect\n";
        return 2;
    }

    char s[INET6_ADDRSTRLEN];
    inet_ntop(current_ai->ai_family, get_in_addr((struct sockaddr *)current_ai->ai_addr),s, sizeof s);
    std::cout << "stash_client: connecting to " << s << "\n";

    freeaddrinfo(gai_result);

    char buf[MAXDATASIZE];
    int numbytes;
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1)
    {
        std::perror("stash_client - recv operation");
        return 1;
    }
    buf[numbytes] = '\0';
    std::cout << "stash_client: received '" << buf <<"'\n";

    close(sockfd);

    return 0;
}

