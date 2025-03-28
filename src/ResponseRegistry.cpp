#include "../include/ResponseRegistry.hpp"

ResponseSlot::ResponseSlot() : ready(false) {}

ResponseRegistry::~ResponseRegistry() {
    for (auto& pair : registry) {
        delete pair.second;
    }
    registry.clear();
}

void ResponseRegistry::registerTrain(int trainId) {
    std::lock_guard<std::mutex> lock(registryMutex);
    if (registry.find(trainId) == registry.end()) {
        registry[trainId] = new ResponseSlot();
    }
}

void ResponseRegistry::unregisterTrain(int trainId) {
    std::lock_guard<std::mutex> lock(registryMutex);
    auto it = registry.find(trainId);
    if (it != registry.end()) {
        delete it->second;
        registry.erase(it);
    }
}

std::vector<unsigned char> ResponseRegistry::waitForResponse(int trainId) {
    ResponseSlot* slot = nullptr;
    {
        std::lock_guard<std::mutex> lock(registryMutex);
        auto it = registry.find(trainId);
        if (it != registry.end()) {
            slot = it->second;
        } else {
            throw std::runtime_error("Train not registered for responses");
        }
    }
    std::unique_lock<std::mutex> lock(slot->mtx);
    slot->cv.wait(lock, [slot]{ return slot->ready; });
    slot->ready = false; // Reset for next message.
    return slot->response;
}

void ResponseRegistry::setResponse(int trainId, const std::vector<unsigned char>& response) {
    ResponseSlot* slot = nullptr;
    {
        std::lock_guard<std::mutex> lock(registryMutex);
        auto it = registry.find(trainId);
        if (it != registry.end()) {
            slot = it->second;
        } else {
            // If the train is not registered, simply return (or log an error).
            return;
        }
    }
    {
        std::lock_guard<std::mutex> lock(slot->mtx);
        slot->response = response;
        slot->ready = true;
    }
    slot->cv.notify_one();
}

ResponseRegistry& ResponseRegistry::instance() {
    static ResponseRegistry instance;
    return instance;
}
