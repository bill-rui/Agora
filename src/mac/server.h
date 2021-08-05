//
// Created by Bill on 7/29/21.
//

#ifndef UDP_SERVER_H
#define UDP_SERVER_H
#include <iostream>
#include <memory>

#include "signal_handler.h"
#include "udp_client.h"
#include "udp_server.h"
class CustomServer {
 private:
  UDPServer server;
  UDPClient sender;
  SignalHandler signal_handler;
  int kMaxRxSize;
  std::string kAddr;
  int kForwardingPort;
  ssize_t kSendPktSize;
  int kBuffSize;
  unsigned char* send_ptr;
  unsigned char* rcv_ptr;
  ssize_t total_rx_data;
  bool print_progress;
  unsigned char* rx_buffer;

 public:
  CustomServer(
      int kBuffSize, int kMaxRxSize, int kServerPort, std::string kAddr,
      int kForwardingPort, ssize_t kSendPktSize
      );
  CustomServer(int kBuffSize, int kMaxRxSize, int kServerPort);
  CustomServer(int kBuffSize, std::string kAddr, int kForwardingPort,
               ssize_t SendPktSize);
  ~CustomServer(){
    delete[] rx_buffer;
  }
  void Forward();
  ssize_t Receive();
  ssize_t WriteTo(void *dest, ssize_t bytes);
  ssize_t Send();
};
void Forward(int kBuffSize, int kMaxRxSize, int kServerPort, std::string kAddr,
             int kForwardingPort, unsigned long kSendPktSize);
void SendData(std::string addr, int port, const unsigned char* buffer, ssize_t len);
void ReceiveData(int port, unsigned char *buffer, ssize_t len);

#endif