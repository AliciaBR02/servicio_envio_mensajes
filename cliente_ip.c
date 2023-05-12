#include <stdio.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <string.h>

int get_interface_address(char *interface_name, char *host, size_t host_len) {
    struct ifaddrs *ifaddr, *ifa;
    int s;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return -1;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        if (strcmp(ifa->ifa_name, interface_name) == 0 && ifa->ifa_addr->sa_family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, host_len, NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                printf("getnameinfo() failed: %s\n", gai_strerror(s));
                freeifaddrs(ifaddr);
                return -1;
            }
            freeifaddrs(ifaddr);
            return 0;
        }
    }

    freeifaddrs(ifaddr);
    return -1;
}

int main() {
    char host[NI_MAXHOST];
    if (get_interface_address("wlp0s20f3", host, NI_MAXHOST) == 0) {
        printf("%s", host);
    }
    return 0;
}
