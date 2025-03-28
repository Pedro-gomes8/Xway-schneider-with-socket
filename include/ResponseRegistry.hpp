#ifndef RESPONSE_REGISTRY_H
#define RESPONSE_REGISTRY_H

#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <stdexcept>

// A slot holding the response for a specific train.
struct ResponseSlot {
    std::mutex mtx;
    std::condition_variable cv;
    bool ready;
    std::vector<unsigned char> response;

    ResponseSlot();
};

class ResponseRegistry {
public:
    // Registers a train ID to receive responses.
    void registerTrain(int trainId);

    void unregisterTrain(int trainId);

    // Blocks until a response is available for the given train ID.
    std::vector<unsigned char> waitForResponse(int trainId);

    // Sets the response for a given train ID and notifies the waiting thread.
    void setResponse(int trainId, const std::vector<unsigned char>& response);

    static ResponseRegistry& instance();

private:
    ResponseRegistry() = default;
    ~ResponseRegistry();


    ResponseRegistry(const ResponseRegistry&) = delete;
    ResponseRegistry& operator=(const ResponseRegistry&) = delete;

    std::mutex registryMutex;
    std::unordered_map<int, ResponseSlot*> registry;
};

#endif // RESPONSE_REGISTRY_H
