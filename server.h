#pragma once
#include "kv_store.h"
#include <atomic>
#include <thread>
#include <vector>
#include <string>

class Server {
public:
    Server(int port, KVStore& store, const std::string& aof_path);
    ~Server();
    void start();
    void stop();

private:
    void accept_loop();
    void handle_client(int fd);
    std::string dispatch(const std::vector<std::string>& args);
    void append_aof(const std::vector<std::string>& args);

    int port_;
    int listen_fd_ = -1;
    KVStore& store_;
    std::string aof_path_;
    std::atomic<bool> running_{false};
    std::vector<std::thread> workers_;
};