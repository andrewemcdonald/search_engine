/*
 * Copyright ©2019 Justin Hsia.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Spring Quarter 2019 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <stdio.h>       // for snprintf()
#include <unistd.h>      // for close(), fcntl()
#include <sys/types.h>   // for socket(), getaddrinfo(), etc.
#include <sys/socket.h>  // for socket(), getaddrinfo(), etc.
#include <arpa/inet.h>   // for inet_ntop()
#include <netdb.h>       // for getaddrinfo()
#include <errno.h>       // for errno, used by strerror()
#include <string.h>      // for memset, strerror()
#include <iostream>      // for std::cerr, etc.

#include "./ServerSocket.h"

#define HOSTNAME_BUFFER_SIZE 128

extern "C" {
  #include "libhw1/CSE333.h"
}

namespace hw4 {

ServerSocket::ServerSocket(uint16_t port) {
  port_ = port;
  listen_sock_fd_ = -1;
}

ServerSocket::~ServerSocket() {
  // Close the listening socket if it's not zero.  The rest of this
  // class will make sure to zero out the socket if it is closed
  // elsewhere.
  if (listen_sock_fd_ != -1)
    close(listen_sock_fd_);
  listen_sock_fd_ = -1;
}

bool ServerSocket::BindAndListen(int ai_family, int *listen_fd) {
  // Use "getaddrinfo," "socket," "bind," and "listen" to
  // create a listening socket on port port_.  Return the
  // listening socket through the output parameter "listen_fd".

  // MISSING:

  int res;

  // Sanity check
  // Verify333(ai_family == AF_INET || ai_family == AF_INET6 ||
            // ai_family == AF_UNSPEC);

  // Set internal field
  sock_family_ = ai_family;

  // Use getaddrinfo() to make a sockaddr for bind()
  struct addrinfo hints, *result;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = sock_family_;
  hints.ai_flags = AI_PASSIVE;  // use wildcard
  res = getaddrinfo(nullptr, std::to_string(port_).c_str(),
                    &hints, &result);
  if (res != 0) {
    // DNS failure
    std::cerr << "DNS Error" << std::endl;
    return false;
  }

  // For debugging only
  // struct sockaddr_in6 *test =
        // reinterpret_cast<struct sockaddr_in6*>(result->ai_addr);

  // Create socket
  listen_sock_fd_ = socket(result->ai_family, result->ai_socktype,
                           result->ai_protocol);
  if (listen_sock_fd_ == -1) {
    //  Error making socket
    perror("socket");
    return false;
  }

  // Bind to specified port.
  res = bind(listen_sock_fd_, result->ai_addr, result->ai_addrlen);
  if (res == -1) {
    // Failed to bind to specified port.
    std::cerr << "bind " << errno << ": ";
    perror(nullptr);
    close(listen_sock_fd_);
    return false;
  }

  // Begin listening
  res = listen(listen_sock_fd_, SOMAXCONN);
  if (res != 0) {
    // Could not get port to listen
    perror("Listen");
    close(listen_sock_fd_);
    return false;
  }

  // Clean up
  freeaddrinfo(result);

  // "Return" file descriptor
  *listen_fd = listen_sock_fd_;

  return true;
}

bool ServerSocket::Accept(int *accepted_fd,
                          std::string *client_addr,
                          uint16_t *client_port,
                          std::string *client_dnsname,
                          std::string *server_addr,
                          std::string *server_dnsname) {
  // Accept a new connection on the listening socket listen_sock_fd_.
  // (Block until a new connection arrives.)  Return the newly accepted
  // socket, as well as information about both ends of the new connection,
  // through the various output parameters.

  // MISSING:
  int res, fd;
  struct sockaddr client_sockaddr;
  socklen_t client_addr_len = 0;

  fd = accept(listen_sock_fd_, &client_sockaddr, &client_addr_len);
  if (fd == -1) {
    // Failed to accept
    return false;
  }

  // "Return" accpted_fd
  *accepted_fd = fd;

  // "Return" client_addr, client_port, and client_dnsname
  if (client_sockaddr.sa_family == AF_INET) {
    // Address is IPV4
    sockaddr_in *in = reinterpret_cast<sockaddr_in *>(&client_sockaddr);

    // "Return client port"
    *client_port = in->sin_port;

    // "Return" client_addr
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &in->sin_addr, str, INET_ADDRSTRLEN);
    *client_addr = str;

    // "Return" client_dnsname
    char cliname[HOSTNAME_BUFFER_SIZE];
    res = getnameinfo(&client_sockaddr, INET_ADDRSTRLEN, cliname,
                      HOSTNAME_BUFFER_SIZE, nullptr, 0, 0);
    if (res == 0) {
      client_dnsname->assign(cliname);
    } else {
      client_dnsname->assign("");
    }
  } else {
    // Address is IPV6
    sockaddr_in6 *in = reinterpret_cast<sockaddr_in6 *>(&client_sockaddr);

    // "Return" client_port
    *client_port = in->sin6_port;

    // "Return" client_addr
    char str[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &in->sin6_addr, str, INET6_ADDRSTRLEN);
    *client_addr = str;

    // "Return" client_dnsname
    char cliname[HOSTNAME_BUFFER_SIZE];
    res = getnameinfo(&client_sockaddr, INET6_ADDRSTRLEN, cliname,
                      HOSTNAME_BUFFER_SIZE, nullptr, 0, 0);
    if (res == 0) {
      client_dnsname->assign(cliname);
    } else {
      client_dnsname->assign("");
    }
  }

  // Get server address info
  struct sockaddr server_sockaddr;
  socklen_t server_sockaddr_len = 0;
  res = getpeername(fd, &server_sockaddr, &server_sockaddr_len);

  if (res == -1) {
    // Failed to retreive info, close socket and return.
    close(listen_sock_fd_);
    return false;
  }

  // "Return" server_addr, server_dnsname
  if (server_sockaddr.sa_family == AF_INET) {
    // Address is IPV4
    sockaddr_in *in = reinterpret_cast<sockaddr_in *>(&server_sockaddr);

    // "Return" server_addr
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &in->sin_addr, str, INET_ADDRSTRLEN);
    *server_addr = str;

    // "Return" server_dnsname
    char servname[HOSTNAME_BUFFER_SIZE];
    res = getnameinfo(&client_sockaddr, INET_ADDRSTRLEN, servname,
                      HOSTNAME_BUFFER_SIZE, nullptr, 0, 0);
    if (res == 0) {
      server_dnsname->assign(servname);
    } else {
      server_dnsname->assign("");
    }

  } else {
    // Address is IPV6
    sockaddr_in6 *in = reinterpret_cast<sockaddr_in6 *>(&server_sockaddr);

    // "Return" server_addr
    char str[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &in->sin6_addr, str, INET6_ADDRSTRLEN);
    *server_addr = str;

    // "Return" server_dnsname
    char servname[HOSTNAME_BUFFER_SIZE];
    res = getnameinfo(&client_sockaddr, INET6_ADDRSTRLEN, servname,
                      HOSTNAME_BUFFER_SIZE, nullptr, 0, 0);
    if (res == 0) {
      server_dnsname->assign(servname);
    } else {
      server_dnsname->assign("");
    }
  }

  // Wipe sweat off brow and return true. ;)
  return true;
}

}  // namespace hw4
