#pragma once

#include <string>
#include <vector>

namespace pebblekv {

// Represents different command types
// Note: We use DEL instead of DELETE because Windows headers define DELETE as a macro
enum class CommandType {
    SET,
    GET,
    DEL,
    EXPIRE,
    TTL,
    UNKNOWN
};

// Parsed command structure
struct Command {
    CommandType type;
    std::vector<std::string> args;
};

class CommandParser {
public:
    // Parse a command string like "SET key value" into a Command struct
    // Commands are case-insensitive
    static Command parse(const std::string& input);

private:
    // Split string by whitespace
    static std::vector<std::string> tokenize(const std::string& input);
    
    // Convert command string to CommandType
    static CommandType getCommandType(const std::string& cmd);
};

} // namespace pebblekv
