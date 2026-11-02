#include "resp_parser.h"
#include <unistd.h>
#include <sstream>

static bool read_line(int fd, std::string& out) {
    out.clear();
    char c;
    while (true) {
        ssize_t n = read(fd, &c, 1);
        if (n <= 0) return false;
        if (c == '\r') {
            char nl;
            if (read(fd, &nl, 1) <= 0 || nl != '\n') return false;
            return true;
        }
        out.push_back(c);
    }
}

std::vector<std::string> RESPParser::read_command(int fd) {
    std::string line;
    if (!read_line(fd, line)) return {};
    if (line.empty() || line[0] != '*') return {};
    
    int argc = std::stoi(line.substr(1));
    std::vector<std::string> args;
    for (int i = 0; i < argc; ++i) {
        if (!read_line(fd, line)) return {};
        if (line.empty() || line[0] != '$') return {};
        int len = std::stoi(line.substr(1));
        std::string arg(len, '\0');
        ssize_t total = 0;
        while (total < len) {
            ssize_t n = read(fd, &arg[total], len - total);
            if (n <= 0) return {};
            total += n;
        }
        char crlf[2];
        if (read(fd, crlf, 2) != 2) return {};
        args.push_back(std::move(arg));
    }
    return args;
}

std::string RESPParser::bulk_string(const std::string& s) {
    return "$" + std::to_string(s.size()) + "\r\n" + s + "\r\n";
}

std::string RESPParser::simple_string(const std::string& s) {
    return "+" + s + "\r\n";
}

std::string RESPParser::integer(long long n) {
    return ":" + std::to_string(n) + "\r\n";
}

std::string RESPParser::null_bulk() { return "$-1\r\n"; }

std::string RESPParser::error(const std::string& msg) {
    return "-" + msg + "\r\n";
}