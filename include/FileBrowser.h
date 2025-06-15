#pragma once
#include <functional>
#include <string>
#include <vector>

class FileBrowser {
public:
  struct FileInfo {
    std::string filename;
    std::string fullPath;
    bool isDirectory;
    size_t fileSize;
    std::string extension;

    FileInfo(const std::string &name, const std::string &path,
             bool isDir = false, size_t size = 0)
        : filename(name), fullPath(path), isDirectory(isDir), fileSize(size) {
      if (!isDir) {
        size_t dotPos = name.find_last_of('.');
        if (dotPos != std::string::npos) {
          extension = name.substr(dotPos + 1);
        }
      }
    }
  };

  // Callback function type for file selection
  using FileSelectedCallback = std::function<void(const std::string &filepath)>;

  // Show native file open dialog
  static std::string
  openFileDialog(const std::string &title = "Open File",
                 const std::string &defaultPath = "",
                 const std::string &filter =
                     "Scene Files (*.scene)|*.scene|All Files (*.*)|*.*");

  // Show native file save dialog
  static std::string
  saveFileDialog(const std::string &title = "Save File",
                 const std::string &defaultPath = "",
                 const std::string &defaultFilename = "",
                 const std::string &filter =
                     "Scene Files (*.scene)|*.scene|All Files (*.*)|*.*");

  // Directory browsing functions
  static std::vector<FileInfo> listDirectory(const std::string &path);
  static std::vector<FileInfo> listSceneFiles(const std::string &directory);
  static bool directoryExists(const std::string &path);
  static bool fileExists(const std::string &path);

  // Path utilities
  static std::string getParentDirectory(const std::string &path);
  static std::string getFileName(const std::string &path);
  static std::string getFileExtension(const std::string &path);
  static std::string joinPaths(const std::string &path1,
                               const std::string &path2);
  static std::string getCurrentDirectory();
  static std::string getSceneDirectory(); // Returns "scenes/" directory

  // File size formatting
  static std::string formatFileSize(size_t bytes);

private:
  // Platform-specific implementations
#ifdef _WIN32
  static std::string openFileDialog_Windows(const std::string &title,
                                            const std::string &defaultPath,
                                            const std::string &filter);
  static std::string saveFileDialog_Windows(const std::string &title,
                                            const std::string &defaultPath,
                                            const std::string &defaultFilename,
                                            const std::string &filter);
#elif defined(__linux__)
  static std::string openFileDialog_Linux(const std::string &title,
                                          const std::string &defaultPath,
                                          const std::string &filter);
  static std::string saveFileDialog_Linux(const std::string &title,
                                          const std::string &defaultPath,
                                          const std::string &defaultFilename,
                                          const std::string &filter);
#elif defined(__APPLE__)
  static std::string openFileDialog_macOS(const std::string &title,
                                          const std::string &defaultPath,
                                          const std::string &filter);
  static std::string saveFileDialog_macOS(const std::string &title,
                                          const std::string &defaultPath,
                                          const std::string &defaultFilename,
                                          const std::string &filter);
#endif

  static std::string
  parseFilter(const std::string &filter); // Convert filter to platform format
};