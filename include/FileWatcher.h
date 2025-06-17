#pragma once
#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// Forward declarations
class Application;

enum class FileType { TEXTURE, SCENE, AUDIO, SHADER, UNKNOWN };

struct WatchedFile {
  std::string path;
  FileType type;
  std::chrono::system_clock::time_point lastModified;
  std::function<void(const std::string &)> reloadCallback;

  WatchedFile()
      : type(FileType::UNKNOWN),
        lastModified(std::chrono::system_clock::now()) {}
  WatchedFile(const std::string &filePath, FileType fileType,
              std::function<void(const std::string &)> callback)
      : path(filePath), type(fileType), reloadCallback(callback),
        lastModified(std::chrono::system_clock::now()) {}
};

class FileWatcher {
private:
  std::unordered_map<std::string, WatchedFile> watchedFiles;
  std::thread watcherThread;
  std::atomic<bool> isRunning;
  mutable std::mutex filesMutex;

  // Polling interval in milliseconds
  static const int POLL_INTERVAL_MS = 500;

  // Reference to application for callbacks
  Application *application;

public:
  FileWatcher();
  ~FileWatcher();

  // Lifecycle
  bool initialize(Application *app);
  void shutdown();

  // File watching
  void watchFile(const std::string &path, FileType type,
                 std::function<void(const std::string &)> reloadCallback);
  void unwatchFile(const std::string &path);
  void clearAllWatches();

  // Manual check for updates
  void checkForUpdates();

  // Enable/disable watching
  void setEnabled(bool enabled);
  bool isEnabled() const { return isRunning.load(); }

  // Statistics
  size_t getWatchedFileCount() const;
  std::vector<std::string> getWatchedFiles() const;

private:
  // Worker thread function
  void watcherLoop();

  // File utilities
  bool fileExists(const std::string &path) const;
  std::chrono::system_clock::time_point
  getFileModificationTime(const std::string &path) const;
  FileType detectFileType(const std::string &path) const;

  // Processing
  void processFileChange(const std::string &path,
                         const WatchedFile &watchedFile);
};