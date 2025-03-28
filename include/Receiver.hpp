#ifndef RECEIVER_HPP
#define RECEIVER_HPP

#include "SocketHandler.hpp"
#include "ResponseRegistry.hpp"
#include <vector>

class Receiver {
public:
    Receiver(SocketHandler* sock);
    
    void start();
    
    void stop();

private:
    SocketHandler* sock;
    bool running;
};

#endif // RECEIVER_HPP
