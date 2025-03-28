#include "Receiver.hpp"
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <errno.h>

Receiver::Receiver(SocketHandler* sock)
    : sock(sock), running(true)
{}

void Receiver::start() {
    const int bufferSize = 256;
    unsigned char buffer[bufferSize];

    while (running) {
        int n = sock->receiveData(buffer, bufferSize);
        if (n > 0) {
            if (buffer[14] == 0xFE) continue;
            // For example, assume the protocol puts the train ID at byte index 18.
            int trainId;
            switch (buffer[18]){
                case 0x08:
                    trainId = 39;
                    break;
                case 0x09:
                    trainId = 39;
                    break;
                case 0x0A:
                    trainId = 49;
                    break;
                case 0x0B:
                    trainId = 49;
                    break;
                case 0x0C:
                    trainId = 52;
                    break;
                case 0x0D:
                    trainId = 52;
                    break;
                case 0x0E:
                    trainId = 52;
                    break;
                case 0x0F:
                    trainId = 42;
                    break;
                case 0x10:
                    trainId= 42;
                    break;
            }
            std::cout << "Receiver: Received " << n << " bytes from train " << trainId << std::endl;

            std::vector<unsigned char> response(buffer, buffer + n);

            // Notify the corresponding train that is waiting for a response.
            ResponseRegistry::instance().setResponse(trainId, response);
        }
        else if (n == 0) {
            std::cout << "Receiver: Connection closed by peer." << std::endl;
            break;
        }
        else {
            std::cerr << "Receiver: Error receiving data: " << strerror(errno) << std::endl;
            break;
        }
    }
}

void Receiver::stop() {
    running = false;
}
