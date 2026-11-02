#include "server.h"
#include "resp_parser.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <fstream>

Server::Server(int port, KVStore& store, const std::string& aof_path)
    : port_(port), store_(store), aof_path_(aof_path) {}

Server::~Server() { stop(); }

void Server::start() {
    listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if (bind(listen_fd_, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return;
    }
    if (listen(listen_fd_, 128) < 0) {
        perror("listen");
        return;
    }

    running_ = true;
    workers_.emplace_back(&Server::accept_loop, this);
    std::cout << "🔥 KV Server listening on port " << port_ << "\n";
}

void Server::stop() {
    running_ = false;
    if (listen_fd_ >= 0) { close(listen_fd_); listen_fd_ = -1; }
    for (auto& t : workers_) if (t.joinable()) t.join();
    workers_.clear();
}

void Server::accept_loop() {
    while (running_) {
        sockaddr_in client{};
        socklen_t len = sizeof(client);
        int fd = accept(listen_fd_, (sockaddr*)&client, &len);
        if (fd < 0) continue;
        // Detached worker thread per client
        std::thread(&Server::handle_client, this, fd).detach();
    }
}

void Server::handle_client(int fd) {
    while (running_) {
        auto args = RESPParser::read_command(fd);
        if (args.empty()) break;
        std::string reply = dispatch(args);
        send(fd, reply.data(), reply.size(), 0);
    }
    close(fd);
}

std::string Server::dispatch(const std::vector<std::string>& args) {
    const std::string& cmd = args[0];
    std::string upper;
    for (char c : cmd) upper.push_back(toupper(c));

    if (upper == "PING") return RESPParser::simple_string("PONG");

    if (upper == "SET" && args.size() == 3) {
        store_.set(args[1], args[2]);
        append_aof(args);
        return RESPParser::simple_string("OK");
    }
    if (upper == "GET" && args.size() == 2) {
        auto v = store_.get(args[1]);
        return v ? RESPParser::bulk_string(*v) : RESPParser::null_bulk();
    }
    if (upper == "DEL" && args.size() == 2) {
        bool ok = store_.del(args[1]);
        append_aof(args);
        return RESPParser::integer(ok ? 1 : 0);
    }
    if (upper == "EXPIRE" && args.size() == 3) {
        bool ok = store_.expire(args[1], std::stoi(args[2]));
        append_aof(args);
        return RESPParser::integer(ok ? 1 : 0);
    }
    if (upper == "TTL" && args.size() == 2) {
        return RESPParser::integer(store_.ttl(args[1]));
    }
    if (upper == "SAVE") {
        store_.save_snapshot("dump.kv");
        return RESPParser::simple_string("OK");
    }
    return RESPParser::error("ERR unknown command");
}

void Server::append_aof(const std::vector<std::string>& args) {
    std::ofstream out(aof_path_, std::ios::app);
    out << "*" << args.size() << "\r\n";
    for (const auto& a : args) {
        out << "$" << a.size() << "\r\n" << a << "\r\n";
    }
}
