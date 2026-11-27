#include "command_parser.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace pebblekv {

Command CommandParser::parse(const std::string& input) {
    Command cmd;
    auto tokens = tokenize(input);
    
    if (tokens.empty()) {
        cmd.type = CommandType::UNKNOWN;
        return cmd;
    }
    
    // First token is the command
    cmd.type = getCommandType(tokens[0]);
    
    // Remaining tokens are arguments
    cmd.args.assign(tokens.begin() + 1, tokens.end());
    
    return cmd;
}

std::vector<std::string> CommandParser::tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::istringstream stream(input);
    std::string token;
    
    // Split by whitespace
    while (stream >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

CommandType CommandParser::getCommandType(const std::string& cmd) {
    // Convert to uppercase for case-insensitive comparison
    std::string upper_cmd = cmd;
    std::transform(upper_cmd.begin(), upper_cmd.end(), upper_cmd.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    
    if (upper_cmd == "SET") {
        return CommandType::SET;
    } else if (upper_cmd == "GET") {
        return CommandType::GET;
    } else if (upper_cmd == "DELETE" || upper_cmd == "DEL") {
        return CommandType::DEL;
    } else if (upper_cmd == "EXPIRE") {
        return CommandType::EXPIRE;
    } else if (upper_cmd == "TTL") {
        return CommandType::TTL;
    }
    
    return CommandType::UNKNOWN;
}

} // namespace pebblekv
