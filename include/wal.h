#pragma once

#include <string>
#include <fstream>

namespace pebblekv {

// Write-Ahead Log for durability
// Records all write operations (SET, DELETE) to disk
// Can replay log on startup to restore database state
class WAL {
public:
    explicit WAL(const std::string& filename);
    ~WAL();
    
    // Open WAL file for writing
    // Returns false if file cannot be opened
    bool open();
    
    // Close WAL file
    void close();
    
    // Log a SET operation
    bool logSet(const std::string& key, const std::string& value);
    
    // Log a DELETE operation
    bool logDelete(const std::string& key);
    
    // Flush pending writes to disk
    void flush();
    
    // Check if WAL is open
    bool isOpen() const { return file_.is_open(); }
    
    // Get the WAL filename
    const std::string& filename() const { return filename_; }

private:
    std::string filename_;
    std::ofstream file_;
    
    // Write a log entry in format: "command key [value]\n"
    bool writeEntry(const std::string& entry);
};

// WAL Replay - reads log file and reconstructs operations
class WALReader {
public:
    explicit WALReader(const std::string& filename);
    ~WALReader();
    
    // Open WAL file for reading
    bool open();
    
    // Close WAL file
    void close();
    
    // Read next entry from log
    // Returns false when end of file or error
    // Sets command, key, and optionally value
    bool readNext(std::string& command, std::string& key, std::string& value);
    
    // Check if file exists
    static bool exists(const std::string& filename);

private:
    std::string filename_;
    std::ifstream file_;
};

} // namespace pebblekv
