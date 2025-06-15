#include "DatabaseManager.h"
#include <filesystem>
#include <iostream>
#include <sqlite3.h>

DatabaseManager::DatabaseManager()
    : m_database(nullptr), m_isConnected(false) {}

DatabaseManager::~DatabaseManager() { close(); }

bool DatabaseManager::initialize(const std::string &databasePath) {
  if (m_isConnected) {
    std::cerr
        << "Database is already connected. Close existing connection first."
        << std::endl;
    return false;
  }

  m_databasePath = databasePath;

  // Create the directory if it doesn't exist
  std::filesystem::path dbPath(databasePath);
  std::filesystem::path dbDir = dbPath.parent_path();

  if (!dbDir.empty() && !std::filesystem::exists(dbDir)) {
    try {
      std::filesystem::create_directories(dbDir);
      std::cout << "Created database directory: " << dbDir << std::endl;
    } catch (const std::filesystem::filesystem_error &ex) {
      std::cerr << "Failed to create database directory: " << ex.what()
                << std::endl;
      return false;
    }
  }

  // Open/create database
  int result = sqlite3_open(databasePath.c_str(), &m_database);

  if (result != SQLITE_OK) {
    std::cerr << "Failed to open database: " << sqlite3_errmsg(m_database)
              << std::endl;
    sqlite3_close(m_database);
    m_database = nullptr;
    return false;
  }

  m_isConnected = true;
  std::cout << "Successfully connected to database: " << databasePath
            << std::endl;

  // Enable foreign key constraints
  const char *enableForeignKeys = "PRAGMA foreign_keys = ON;";
  char *errorMessage = nullptr;
  result = sqlite3_exec(m_database, enableForeignKeys, nullptr, nullptr,
                        &errorMessage);

  if (result != SQLITE_OK) {
    std::cerr << "Failed to enable foreign keys: " << errorMessage << std::endl;
    sqlite3_free(errorMessage);
    // Don't fail initialization for this, just warn
  }

  return true;
}

void DatabaseManager::close() {
  if (m_database) {
    sqlite3_close(m_database);
    m_database = nullptr;
    m_isConnected = false;
    std::cout << "Database connection closed." << std::endl;
  }
}

bool DatabaseManager::isConnected() const { return m_isConnected; }

const std::string &DatabaseManager::getDatabasePath() const {
  return m_databasePath;
}