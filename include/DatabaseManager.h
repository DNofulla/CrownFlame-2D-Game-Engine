#pragma once

#include <memory>
#include <string>

// Forward declaration to avoid including sqlite3.h in header
struct sqlite3;

class DatabaseManager {
public:
  DatabaseManager();
  ~DatabaseManager();

  // Initialize database connection
  bool initialize(const std::string &databasePath);

  // Close database connection
  void close();

  // Check if database is connected
  bool isConnected() const;

  // Get the database path
  const std::string &getDatabasePath() const;

private:
  sqlite3 *m_database;
  std::string m_databasePath;
  bool m_isConnected;

  // Disable copy constructor and assignment operator
  DatabaseManager(const DatabaseManager &) = delete;
  DatabaseManager &operator=(const DatabaseManager &) = delete;
};