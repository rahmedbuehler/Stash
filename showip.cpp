/*
** showip.cpp -- show IP addresses for a host given on the command line
*/

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char* argv[])
{
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: showip hostname\n");
        return 1;
    }


    // <addrinfo> struct is used to prep socket address structures for use along with host name and service name lookups.  Members start with "ai_".  <ai_addr> is a pointer to a <sockaddr> struct, members of this start with "sa_"
    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo * result; // For storing the result of getaddrinfo

    // getaddrinfo takes a node (site name or ip), service (e.g., http or port number), addrinfo struct pointer, and generates a linked-list of addrinfo structs as a result
    if ((status = getaddrinfo(argv[1], NULL, &hints, &result)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    printf("IP addresses for %s:\n\n", argv[1]);

    for(struct addrinfo * p = result;p != NULL; p = p->ai_next) {
        void* addr;
        char* ipver;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in * ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        } else { // IPv6
            struct sockaddr_in6 * ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert the IP to a string and print it:
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("  %s: %s\n", ipver, ipstr);
    }

    freeaddrinfo(result); // free the (dynamically-allocated) linked list

    return 0;
}
