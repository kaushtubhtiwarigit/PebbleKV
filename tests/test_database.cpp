#include <gtest/gtest.h>
#include "database.h"
#include "command_parser.h"

using namespace pebblekv;

// Test fixture for Database tests
class DatabaseTest : public ::testing::Test {
protected:
    Database db;
};

// Basic SET and GET operations
TEST_F(DatabaseTest, SetAndGet) {
    EXPECT_TRUE(db.set("name", "Kaushtubh"));
    
    auto value = db.get("name");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "Kaushtubh");
}

// GET on non-existent key
TEST_F(DatabaseTest, GetNonExistent) {
    auto value = db.get("nonexistent");
    EXPECT_FALSE(value.has_value());
}

// Update existing key
TEST_F(DatabaseTest, UpdateExistingKey) {
    db.set("counter", "1");
    db.set("counter", "2");
    
    auto value = db.get("counter");
    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, "2");
}

// DELETE operation
TEST_F(DatabaseTest, DeleteKey) {
    db.set("temp", "value");
    EXPECT_TRUE(db.exists("temp"));
    
    bool deleted = db.del("temp");
    EXPECT_TRUE(deleted);
    EXPECT_FALSE(db.exists("temp"));
    
    // Deleting again should return false
    deleted = db.del("temp");
    EXPECT_FALSE(deleted);
}

// Size tracking
TEST_F(DatabaseTest, Size) {
    EXPECT_EQ(db.size(), 0);
    
    db.set("key1", "value1");
    EXPECT_EQ(db.size(), 1);
    
    db.set("key2", "value2");
    EXPECT_EQ(db.size(), 2);
    
    // Updating existing key shouldn't change size
    db.set("key1", "newvalue");
    EXPECT_EQ(db.size(), 2);
    
    db.del("key1");
    EXPECT_EQ(db.size(), 1);
}

// Clear all data
TEST_F(DatabaseTest, Clear) {
    db.set("key1", "value1");
    db.set("key2", "value2");
    EXPECT_EQ(db.size(), 2);
    
    db.clear();
    EXPECT_EQ(db.size(), 0);
    EXPECT_FALSE(db.exists("key1"));
    EXPECT_FALSE(db.exists("key2"));
}

// Empty keys and values
TEST_F(DatabaseTest, EmptyStrings) {
    db.set("", "empty_key");
    db.set("empty_value", "");
    
    auto value1 = db.get("");
    ASSERT_TRUE(value1.has_value());
    EXPECT_EQ(*value1, "empty_key");
    
    auto value2 = db.get("empty_value");
    ASSERT_TRUE(value2.has_value());
    EXPECT_EQ(*value2, "");
}

// Test CommandParser
TEST(CommandParserTest, ParseSetCommand) {
    auto cmd = CommandParser::parse("SET name John");
    EXPECT_EQ(cmd.type, CommandType::SET);
    ASSERT_EQ(cmd.args.size(), 2);
    EXPECT_EQ(cmd.args[0], "name");
    EXPECT_EQ(cmd.args[1], "John");
}

TEST(CommandParserTest, ParseGetCommand) {
    auto cmd = CommandParser::parse("GET key");
    EXPECT_EQ(cmd.type, CommandType::GET);
    ASSERT_EQ(cmd.args.size(), 1);
    EXPECT_EQ(cmd.args[0], "key");
}

TEST(CommandParserTest, ParseDeleteCommand) {
    auto cmd = CommandParser::parse("DELETE key");
    EXPECT_EQ(cmd.type, CommandType::DELETE);
    ASSERT_EQ(cmd.args.size(), 1);
    EXPECT_EQ(cmd.args[0], "key");
    
    // Test DEL alias
    cmd = CommandParser::parse("DEL key");
    EXPECT_EQ(cmd.type, CommandType::DELETE);
}

TEST(CommandParserTest, CaseInsensitive) {
    auto cmd1 = CommandParser::parse("set key value");
    EXPECT_EQ(cmd1.type, CommandType::SET);
    
    auto cmd2 = CommandParser::parse("SeT key value");
    EXPECT_EQ(cmd2.type, CommandType::SET);
    
    auto cmd3 = CommandParser::parse("SET key value");
    EXPECT_EQ(cmd3.type, CommandType::SET);
}

TEST(CommandParserTest, UnknownCommand) {
    auto cmd = CommandParser::parse("INVALID key");
    EXPECT_EQ(cmd.type, CommandType::UNKNOWN);
}

TEST(CommandParserTest, EmptyInput) {
    auto cmd = CommandParser::parse("");
    EXPECT_EQ(cmd.type, CommandType::UNKNOWN);
    EXPECT_TRUE(cmd.args.empty());
}

TEST(CommandParserTest, MultipleSpaces) {
    auto cmd = CommandParser::parse("SET    key    value");
    EXPECT_EQ(cmd.type, CommandType::SET);
    ASSERT_EQ(cmd.args.size(), 2);
    EXPECT_EQ(cmd.args[0], "key");
    EXPECT_EQ(cmd.args[1], "value");
}
