#include "wal.h"
#include <iostream>
#include <sstream>
#include <cstdio>

#ifdef _WIN32
#include <io.h>
#define access _access
#define F_OK 0
#else
#include <unistd.h>
#endif

namespace pebblekv {

WAL::WAL(const std::string& filename) : filename_(filename) {}

WAL::~WAL() {
    close();
}

bool WAL::open() {
    file_.open(filename_, std::ios::out | std::ios::app);
    if (!file_.is_open()) {
        std::cerr << "Failed to open WAL file: " << filename_ << "\n";
        return false;
    }
    return true;
}

void WAL::close() {
    if (file_.is_open()) {
        file_.flush();
        file_.close();
    }
}

bool WAL::logSet(const std::string& key, const std::string& value) {
    // Format: SET key value
    std::string entry = "SET " + key + " " + value + "\n";
    return writeEntry(entry);
}

bool WAL::logDelete(const std::string& key) {
    // Format: DEL key
    std::string entry = "DEL " + key + "\n";
    return writeEntry(entry);
}

void WAL::flush() {
    if (file_.is_open()) {
        file_.flush();
    }
}

bool WAL::writeEntry(const std::string& entry) {
    if (!file_.is_open()) {
        return false;
    }
    
    file_ << entry;
    file_.flush();  // Flush immediately for durability
    
    return file_.good();
}

// WALReader implementation

WALReader::WALReader(const std::string& filename) : filename_(filename) {}

WALReader::~WALReader() {
    close();
}

bool WALReader::open() {
    file_.open(filename_, std::ios::in);
    if (!file_.is_open()) {
        return false;
    }
    return true;
}

void WALReader::close() {
    if (file_.is_open()) {
        file_.close();
    }
}

bool WALReader::readNext(std::string& command, std::string& key, std::string& value) {
    if (!file_.is_open() || file_.eof()) {
        return false;
    }
    
    std::string line;
    if (!std::getline(file_, line)) {
        return false;
    }
    
    // Skip empty lines
    if (line.empty()) {
        return readNext(command, key, value);
    }
    
    // Parse: "COMMAND key [value]"
    std::istringstream iss(line);
    
    if (!(iss >> command)) {
        return false;
    }
    
    if (!(iss >> key)) {
        return false;
    }
    
    // Value is optional (for DEL commands)
    value.clear();
    if (command == "SET") {
        // Read rest of line as value (may contain spaces)
        std::getline(iss, value);
        // Trim leading space
        if (!value.empty() && value[0] == ' ') {
            value = value.substr(1);
        }
    }
    
    return true;
}

bool WALReader::exists(const std::string& filename) {
    return access(filename.c_str(), F_OK) == 0;
}

} // namespace pebblekv
