#include <iostream>
#include <cstdio> //perror
#include <stdlib.h> //perror
#include <unistd.h> //fork
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, nullptr, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void * get_in_addr(struct sockaddr * sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int setup_server ()
{
    struct addrinfo hints {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    struct addrinfo * gai_server_info;
    const char * port {"3490"};
    int gai_return_value {getaddrinfo(nullptr, port, &hints, &gai_server_info)};
    if (gai_return_value != 0)
    {
        throw gai_strerror(gai_return_value);
    }

    // loop through all the <gai_server_info> results and bind to the first we can
    int yes {1};
    int listen_sockfd;
    struct addrinfo * current_ai;
    for(current_ai = gai_server_info; current_ai != nullptr; current_ai = current_ai->ai_next)
    {
        listen_sockfd = socket(current_ai->ai_family, current_ai->ai_socktype, current_ai->ai_protocol);
        if ( listen_sockfd == -1)
        {
            std::perror("stash_server - socket");
            continue;
        }

        if (setsockopt(listen_sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
        {
            std::perror("stash_server - setsockopt");
            throw "stash_server: setsockopt operation failed in setup_server";
        }

        if (bind(listen_sockfd, current_ai->ai_addr, current_ai->ai_addrlen) == -1)
        {
            close(listen_sockfd);
            std::perror("stash_server - bind");
            continue;
        }

        break;
    }

    freeaddrinfo(gai_server_info);

    if (current_ai == nullptr)
    {
        throw "stash_server: bind operation failed in setup_server";
    }

    int backlog {10};
    if (listen(listen_sockfd, backlog) == -1)
    {
        std::perror("stash_server - listen");
        throw "stash_server: listen operation failed in setup_server";
    }

    return listen_sockfd;
}

int accept_client(const int listen_sockfd)
{
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size {sizeof their_addr};
    int client_sockfd {accept(listen_sockfd, (sockaddr*)(&their_addr), &sin_size)};
    if (client_sockfd == -1)
    {
        return -1;
    }

    char ip_string[INET6_ADDRSTRLEN];
    inet_ntop(their_addr.ss_family, get_in_addr((sockaddr *)(&their_addr)), ip_string, sizeof ip_string);
    std::cout << "stash_server: got connection from " << ip_string << "/n";
    return client_sockfd;
}

int main()
{
    try
    {
        int listen_sockfd {setup_server()};

        struct sigaction sa;
        sa.sa_handler = sigchld_handler; // reap all dead processes
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGCHLD, &sa, nullptr) == -1)
        {
            std::perror("stash_server - sigaction");
            throw "stash_server: sigaction failed";
        }
        std::cout << "stash_server: waiting for connections...\n";

        while(true)  // main accept() loop
        {
            int client_sockfd {accept_client(listen_sockfd)};
            if (client_sockfd == -1)
            {
                continue;
            }
            if (!fork())// fork with child handling talking to this client and parent listening
            { // this is the child process
                close(listen_sockfd);
                if (send(client_sockfd, "Hello, world!", 13, 0) == -1)
                {
                    std::perror("stash_server - send");
                }
                close(listen_sockfd);
                return 0;
            }
            close(client_sockfd);  // parent doesn't need this
        }
        close(listen_sockfd);
    }
    catch(...)
    {
        std::cout << "stash_server: an unexpected exception occurred.\n";
    }
    return 0;
}
