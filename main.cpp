#include "server.h"
#include "kv_store.h"
#include <iostream>
#include <csignal>

static Server* g_server = nullptr;
static void on_sigint(int) { if (g_server) g_server->stop(); }

int main(int argc, char** argv) {
    int port = argc > 1 ? std::stoi(argv[1]) : 6379;

    KVStore store;
    store.load_snapshot("dump.kv");   // restore from previous run

    Server server(port, store, "appendonly.aof");
    g_server = &server;
    std::signal(SIGINT, on_sigint);

    server.start();

    // Block main thread
    while (true) std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
