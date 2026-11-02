#pragma once
#include <string>
#include <vector>

// Parses a single RESP command like: *2\r\n$3\r\nGET\r\n$4\r\nname\r\n
// Returns vector of args. Empty vector on parse error.
class RESPParser {
public:
    // Blocks on socket, reads exactly one command
    static std::vector<std::string> read_command(int fd);
    
    // Serialize a simple string reply
    static std::string bulk_string(const std::string& s);
    static std::string simple_string(const std::string& s);
    static std::string integer(long long n);
    static std::string null_bulk();
    static std::string error(const std::string& msg);
};