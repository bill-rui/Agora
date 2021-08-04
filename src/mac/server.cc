#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

#include "server.h"

#include <iostream>

#include "signal_handler.h"
#include "udp_client.h"
#include "udp_server.h"
#pragma clang diagnostic push

/**
 * Constructor for server that sends to another address
 * @param kBuffSize buffer size
 * @param kMaxRxSize max receive size
 * @param kServerPort port of server
 * @param kAddr client address
 * @param kForwardingPort client port
 * @param kSendPktSize packet size
 */
CustomServer::CustomServer(const int kBuffSize, const int kMaxRxSize,
                               const int kServerPort, const std::string kAddr,
                               const int kForwardingPort,
                               const ssize_t kSendPktSize)
    :server(kServerPort, kBuffSize){
  this->rx_buffer = new unsigned char[kBuffSize + kMaxRxSize];
  this->kAddr = kAddr;
  this->kMaxRxSize = kMaxRxSize;
  this->kForwardingPort = kForwardingPort;
  this->kBuffSize = kBuffSize;
  this->kSendPktSize = kSendPktSize;
  this->send_ptr = rx_buffer;
  this->rcv_ptr = send_ptr;
  this->total_rx_data = 0;
  this->print_progress = false;  // TODO remove
  server.MakeBlocking(1);
}

/**
 * Constructor for server that doesn't send to an ip address
 * @param kBuffSize buffer size
 * @param kMaxRxSize max receive size
 * @param kServerPort server port
 */
CustomServer::CustomServer(
    int kBuffSize, int kMaxRxSize, int kServerPort
    ):server(kServerPort, kBuffSize){
  this->rx_buffer = new unsigned char[kBuffSize + kMaxRxSize];
  this->kAddr = "";
  this->kMaxRxSize = kMaxRxSize;
  this->kBuffSize = kBuffSize;
  this->send_ptr = rx_buffer;
  this->rcv_ptr = send_ptr;
  this->total_rx_data = 0;
  this->print_progress = true;  // TODO remove
  server.MakeBlocking(1);
}

/**
 * Forward UDP packets to the given address and port in given sizes
 */
void CustomServer::Forward(){
  signal_handler.SetupSignalHandlers();

  std::cout << "Forwarding to address: " << kAddr << ":" << kForwardingPort
            << "\n";
  std::cout << "Sending packet size: " << kSendPktSize << " bytes" << std::endl;

  while (!SignalHandler::GotExitSignal()) {
    Receive();
    while (Send() >= 0);
  }
}

/**
 * Receive packet to buffer
 * @return Size of packet received
 */
ssize_t CustomServer::Receive() {
    if (rcv_ptr + kMaxRxSize - reinterpret_cast<const uint8_t *>(rx_buffer) >
      kBuffSize){
        memcpy(rx_buffer, send_ptr, total_rx_data);
        rcv_ptr = reinterpret_cast<uint8_t *>(rx_buffer) + total_rx_data;
        send_ptr = reinterpret_cast<uint8_t *>(rx_buffer);
    }

    ssize_t received = server.Recv(rcv_ptr, kBuffSize);
    if (print_progress){ // TODO remove
      std::cout << "packet received: " << received << std::endl;
    }
    if (received < 0){
        throw std::runtime_error("Receive error");
    }
    else if (received > kMaxRxSize){
        throw std::runtime_error("Received packet larger than max receive size, "
          "check input");
    }
    total_rx_data += received;
    rcv_ptr += received;
    return received;
}

/**
 * Write given bytes from buffer to the destination
 * @param dest pointer to destination
 * @param bytes number of bytes to write
 * @return number of bytes written, -1 if there isn't enough data in buffer
 */
ssize_t CustomServer::WriteTo(void *dest, ssize_t bytes){
    if (bytes > total_rx_data){ Receive(); }
    memcpy(dest, send_ptr, bytes);
    total_rx_data -= bytes;
    send_ptr += bytes;
    return bytes;
}

/**
 * Send a packet
 * @return size of packet sent, -1 if there isn't enough data
 */
ssize_t CustomServer::Send(){
  ssize_t sent;
  if (kSendPktSize > total_rx_data){ return -1; }

  try {
    sent = sender.Send(kAddr, kForwardingPort, send_ptr, kSendPktSize);
    if (print_progress) // TODO remove
      std::cout << "packet sent: " << sent << std::endl;
  }
  catch (std::runtime_error &e) {
    std::cout << "Sending error: " << e.what() << std::endl;
  }

  send_ptr += sent;
  total_rx_data -= sent;
  return sent;
}


#pragma clang diagnostic pop