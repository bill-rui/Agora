//
// Created by Bill on 2/25/22.
//

#include <SoapySDR/Device.hpp>
#include "stream.h"
#include <sys/socket.h>  // Bypass testing
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>
#include "comms-lib.h"


int Stream::setupStream(SoapySDR::Device *remote) {
    int ret = 0;
    printf("[DEBUG] point 0\n");
    auto remoteIPv6Addr = remote->readSetting("ETH0_IPv6_ADDR");
    const auto remoteServPort = remote->readSetting("UDP_SERVICE_PORT");
    const auto rfTxFifoDepth = std::stoul(remote->
            readSetting("RF_TX_FIFO_DEPTH"));

    // scope id change. only applies to vulture
    if (!remoteIPv6Addr.empty()) {
        uint64_t idx = remoteIPv6Addr.find('%');
        remoteIPv6Addr = remoteIPv6Addr.substr(0, idx);
        remoteIPv6Addr.append("%5");
    } else {
        fprintf(stderr, "IPv6 Address empty");
        return(-1);
    }

    int sock = ::socket(AF_INET6, SOCK_DGRAM, 0);
    if (sock == -1) {
        fprintf(stderr, "sock initialization error");
        return sock;
    }
    int one = 1;
    if ((ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                          (const char *)&one, sizeof(one))) != 0) {
        fprintf(stderr, "setsockopt() error\n");
        return ret;
    }
    printf("[DEBUG] point 1\n");
    std::unique_ptr<sockaddr_in6> addr(new sockaddr_in6);
    addr->sin6_family = AF_INET6;
    if ((ret = ::bind(sock, (struct sockaddr *)addr.get(),
                      sizeof(struct sockaddr_in6))) != 0) {
        fprintf(stderr, "bind() error\n");
        return ret;
    }
    // scope id???
    int remote_sock = socket(AF_INET6, SOCK_DGRAM, 0);
    if (remote_sock == -1) {
        fprintf(stderr, "sock initialization error");
        return remote_sock;
    }
    printf("[DEBUG] point 2\n");
    std::unique_ptr<sockaddr_in6> remote_addr(new sockaddr_in6);
    remote_addr->sin6_family = AF_INET6;
    remote_addr->sin6_port = htons(stoi(remoteServPort));
    inet_pton(AF_INET6, remoteIPv6Addr.c_str(), &(remote_addr->sin6_addr));
    if ((ret = ::connect(remote_sock, (struct sockaddr *)remote_addr.get(),
                         sizeof(*remote_addr))) != 0) {
        fprintf(stderr, "connect() error\n");
    }

    printf("[DEBUG] point 3\n");

    return ret;
}