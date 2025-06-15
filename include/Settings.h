#pragma once
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

class Settings {
private:
  std::string settingsFile;
  std::map<std::string, std::string> settings;

  void createDefaultSettings();
  bool loadFromFile();
  bool saveToFile();

  // Helper functions for type conversion
  template <typename T> std::string toString(const T &value);

  template <typename T>
  T fromString(const std::string &str, const T &defaultValue);

public:
  Settings(const std::string &filename = "resources/settings.cfg");
  ~Settings();

  // Monitor settings
  void saveMonitorSettings(GLFWwindow *window);
  bool restoreMonitorSettings(GLFWwindow *window);

  // Window settings
  void saveWindowSettings(int width, int height, int xPos, int yPos);
  bool getWindowSettings(int &width, int &height, int &xPos, int &yPos);

  // Get monitor information
  int getLastMonitorIndex();
  void setLastMonitorIndex(int monitorIndex);

  // Get window position and size from current window
  void saveCurrentWindowState(GLFWwindow *window);

  // General settings management
  void save();
  void load();

  // Get/Set any setting
  template <typename T> void setSetting(const std::string &key, const T &value);

  template <typename T>
  T getSetting(const std::string &key, const T &defaultValue);
};

// Template implementations
template <typename T> std::string Settings::toString(const T &value) {
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

// Specialization for bool
template <> inline std::string Settings::toString<bool>(const bool &value) {
  return value ? "true" : "false";
}

template <typename T>
T Settings::fromString(const std::string &str, const T &defaultValue) {
  std::istringstream iss(str);
  T value;
  if (iss >> value) {
    return value;
  }
  return defaultValue;
}

// Specialization for bool
template <>
inline bool Settings::fromString<bool>(const std::string &str,
                                       const bool &defaultValue) {
  if (str == "true" || str == "1")
    return true;
  if (str == "false" || str == "0")
    return false;
  return defaultValue;
}

template <typename T>
void Settings::setSetting(const std::string &key, const T &value) {
  settings[key] = toString(value);
}

template <typename T>
T Settings::getSetting(const std::string &key, const T &defaultValue) {
  auto it = settings.find(key);
  if (it != settings.end()) {
    return fromString(it->second, defaultValue);
  }
  return defaultValue;
}