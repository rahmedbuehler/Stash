#include <iostream>
#include <cstdio> //perror
#include <cstdlib> //perror, exit
#include <unistd.h> //fork
#include <cerrno>
#include <cstring>
#include <sys/types.h> //waitpid, fork
#include <sys/socket.h> //sockaddr
#include <netinet/in.h> //sockaddr_in
#include <netdb.h> //addrinfo
#include <arpa/inet.h> // in_addr
#include <sys/wait.h> //waitpid
#include <csignal> //sigaction, all signal names

// Handler for child process termination signal
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, nullptr, WNOHANG) > 0);

    errno = saved_errno;
}


// Returns <sin6_addr> or <sin_addr> depending on <sa>
void * get_in_addr(struct sockaddr * sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Returns sockfd that server is listening on
int setup_server ()
{
    struct addrinfo hints {0};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    // Get linked list of address info
    struct addrinfo * gai_server_info;
    const char * port {"3490"};
    int gai_return_value {getaddrinfo(nullptr, port, &hints, &gai_server_info)};
    if (gai_return_value != 0)
    {
        throw gai_strerror(gai_return_value);
    }

    // Loop through all the <gai_server_info> results and bind to the first we can
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

// Outputs connection info and returns client's socket file descriptor
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
    std::cout << "stash_server: got connection from " << ip_string << "\n";
    return client_sockfd;
}

void send_file (const int client_sockfd)
{
    int attempt_num = 0;
    constexpr int max_attempts {10};
    while (attempt_num < max_attempts)
    {
        long int send_result {send(client_sockfd, "Hello, world!", 13, 0)};
        if (send_result == -1)
        {
            std::cerr << "stash_server - send attempt failed (" << attempt_num << " of " << max_attempts <<")\n";
            std::perror("");
            ++attempt_num;
            continue;
        }
        break;
    }
    if (attempt_num < max_attempts)
    {
        std::cout << "stash_server: successful send\n";
    }
    else
    {
        std::cout << "stash_server: unsuccessful send\n";
    }
}

int main()
{
    try
    {
        int listen_sockfd {setup_server()};

        // Handle child process termination signal
        struct sigaction sa;
        sa.sa_handler = sigchld_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        if (sigaction(SIGCHLD, &sa, nullptr) == -1)
        {
            std::perror("stash_server - sigaction");
            throw "stash_server: sigaction failed";
        }

        // Accept loop
        std::cout << "stash_server: waiting for connections...\n";
        while(true)
        {
            int client_sockfd {accept_client(listen_sockfd)};
            if (client_sockfd == -1)
            {
                continue;
            }
            pid_t fork_pid {fork()};
            switch(fork_pid)
            {
                case -1: // Fork failed
                    std::cerr << "stash_server: Failed to fork\n";
                    break;
                case 0: // Child
                    close(listen_sockfd);
                    send_file(client_sockfd);
                    close(client_sockfd);
                    exit(0);
                default: // Parent
                    break;
            }
            close(client_sockfd);
        }
        close(listen_sockfd);
    }
    catch(...)
    {
        std::cout << "stash_server: an unexpected exception occurred.\n";
    }
    return 0;
}
