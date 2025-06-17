#include "FileWatcher.h"
#include "Application.h"
#include <algorithm>
#include <filesystem>
#include <iostream>


FileWatcher::FileWatcher() : isRunning(false), application(nullptr) {}

FileWatcher::~FileWatcher() { shutdown(); }

bool FileWatcher::initialize(Application *app) {
  if (!app) {
    std::cerr << "FileWatcher: Application pointer is null!" << std::endl;
    return false;
  }

  application = app;
  isRunning = true;

  // Start the watcher thread
  watcherThread = std::thread(&FileWatcher::watcherLoop, this);

  std::cout << "FileWatcher: Initialized and started monitoring" << std::endl;
  return true;
}

void FileWatcher::shutdown() {
  if (isRunning.load()) {
    isRunning = false;

    // Wait for thread to finish
    if (watcherThread.joinable()) {
      watcherThread.join();
    }

    clearAllWatches();
    application = nullptr;

    std::cout << "FileWatcher: Shutdown complete" << std::endl;
  }
}

void FileWatcher::watchFile(
    const std::string &path, FileType type,
    std::function<void(const std::string &)> reloadCallback) {
  std::lock_guard<std::mutex> lock(filesMutex);

  if (!fileExists(path)) {
    std::cerr << "FileWatcher: File does not exist: " << path << std::endl;
    return;
  }

  // Auto-detect file type if unknown
  if (type == FileType::UNKNOWN) {
    type = detectFileType(path);
  }

  auto modTime = getFileModificationTime(path);
  watchedFiles[path] = WatchedFile(path, type, reloadCallback);
  watchedFiles[path].lastModified = modTime;

  std::cout << "FileWatcher: Now watching " << path << std::endl;
}

void FileWatcher::unwatchFile(const std::string &path) {
  std::lock_guard<std::mutex> lock(filesMutex);

  auto it = watchedFiles.find(path);
  if (it != watchedFiles.end()) {
    watchedFiles.erase(it);
    std::cout << "FileWatcher: Stopped watching " << path << std::endl;
  }
}

void FileWatcher::clearAllWatches() {
  std::lock_guard<std::mutex> lock(filesMutex);
  watchedFiles.clear();
  std::cout << "FileWatcher: Cleared all watches" << std::endl;
}

void FileWatcher::checkForUpdates() {
  std::lock_guard<std::mutex> lock(filesMutex);

  for (auto &[path, watchedFile] : watchedFiles) {
    if (!fileExists(path)) {
      std::cout << "FileWatcher: File no longer exists: " << path << std::endl;
      continue;
    }

    auto currentModTime = getFileModificationTime(path);
    if (currentModTime > watchedFile.lastModified) {
      std::cout << "FileWatcher: Detected change in " << path << std::endl;
      processFileChange(path, watchedFile);
      watchedFile.lastModified = currentModTime;
    }
  }
}

void FileWatcher::setEnabled(bool enabled) {
  if (enabled && !isRunning.load()) {
    isRunning = true;
    watcherThread = std::thread(&FileWatcher::watcherLoop, this);
    std::cout << "FileWatcher: Enabled" << std::endl;
  } else if (!enabled && isRunning.load()) {
    isRunning = false;
    if (watcherThread.joinable()) {
      watcherThread.join();
    }
    std::cout << "FileWatcher: Disabled" << std::endl;
  }
}

size_t FileWatcher::getWatchedFileCount() const {
  std::lock_guard<std::mutex> lock(filesMutex);
  return watchedFiles.size();
}

std::vector<std::string> FileWatcher::getWatchedFiles() const {
  std::lock_guard<std::mutex> lock(filesMutex);
  std::vector<std::string> files;
  files.reserve(watchedFiles.size());

  for (const auto &[path, _] : watchedFiles) {
    files.push_back(path);
  }

  return files;
}

void FileWatcher::watcherLoop() {
  while (isRunning.load()) {
    checkForUpdates();
    std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
  }
}

bool FileWatcher::fileExists(const std::string &path) const {
  return std::filesystem::exists(path);
}

std::chrono::system_clock::time_point
FileWatcher::getFileModificationTime(const std::string &path) const {
  try {
    auto ftime = std::filesystem::last_write_time(path);
    auto sctp =
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - std::filesystem::file_time_type::clock::now() +
            std::chrono::system_clock::now());
    return sctp;
  } catch (const std::exception &e) {
    std::cerr << "FileWatcher: Error getting modification time for " << path
              << ": " << e.what() << std::endl;
    return std::chrono::system_clock::now();
  }
}

FileType FileWatcher::detectFileType(const std::string &path) const {
  std::string extension = std::filesystem::path(path).extension().string();
  std::transform(extension.begin(), extension.end(), extension.begin(),
                 ::tolower);

  if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
      extension == ".bmp" || extension == ".tga") {
    return FileType::TEXTURE;
  } else if (extension == ".scene") {
    return FileType::SCENE;
  } else if (extension == ".mp3" || extension == ".wav" ||
             extension == ".ogg") {
    return FileType::AUDIO;
  } else if (extension == ".glsl" || extension == ".vert" ||
             extension == ".frag") {
    return FileType::SHADER;
  }

  return FileType::UNKNOWN;
}

void FileWatcher::processFileChange(const std::string &path,
                                    const WatchedFile &watchedFile) {
  if (watchedFile.reloadCallback) {
    // Add a small delay to ensure file is fully written
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    try {
      watchedFile.reloadCallback(path);
      std::cout << "FileWatcher: Successfully reloaded " << path << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "FileWatcher: Error reloading " << path << ": " << e.what()
                << std::endl;
    }
  }
}