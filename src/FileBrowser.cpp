#include "FileBrowser.h"
#include <algorithm>
#include <filesystem>
#include <iostream>

#ifdef _WIN32
#include <cstring>

#elif defined(__linux__)
#include <cstdlib>
#include <unistd.h>
#elif defined(__APPLE__)
#include <cstdlib>
#endif

std::string FileBrowser::openFileDialog(const std::string &title,
                                        const std::string &defaultPath,
                                        const std::string &filter) {
#ifdef _WIN32
  return openFileDialog_Windows(title, defaultPath, filter);
#elif defined(__linux__)
  return openFileDialog_Linux(title, defaultPath, filter);
#elif defined(__APPLE__)
  return openFileDialog_macOS(title, defaultPath, filter);
#else
  std::cerr << "File dialog not supported on this platform" << std::endl;
  return "";
#endif
}

std::string FileBrowser::saveFileDialog(const std::string &title,
                                        const std::string &defaultPath,
                                        const std::string &defaultFilename,
                                        const std::string &filter) {
#ifdef _WIN32
  return saveFileDialog_Windows(title, defaultPath, defaultFilename, filter);
#elif defined(__linux__)
  return saveFileDialog_Linux(title, defaultPath, defaultFilename, filter);
#elif defined(__APPLE__)
  return saveFileDialog_macOS(title, defaultPath, defaultFilename, filter);
#else
  std::cerr << "File dialog not supported on this platform" << std::endl;
  return "";
#endif
}

std::vector<FileBrowser::FileInfo>
FileBrowser::listDirectory(const std::string &path) {
  std::vector<FileInfo> files;

  try {
    if (!std::filesystem::exists(path) ||
        !std::filesystem::is_directory(path)) {
      return files;
    }

    for (const auto &entry : std::filesystem::directory_iterator(path)) {
      std::string filename = entry.path().filename().string();
      std::string fullPath = entry.path().string();
      bool isDir = entry.is_directory();
      size_t fileSize = isDir ? 0 : std::filesystem::file_size(entry);

      files.emplace_back(filename, fullPath, isDir, fileSize);
    }

    // Sort: directories first, then files, both alphabetically
    std::sort(files.begin(), files.end(),
              [](const FileInfo &a, const FileInfo &b) {
                if (a.isDirectory != b.isDirectory) {
                  return a.isDirectory > b.isDirectory;
                }
                return a.filename < b.filename;
              });

  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "Error listing directory: " << e.what() << std::endl;
  }

  return files;
}

std::vector<FileBrowser::FileInfo>
FileBrowser::listSceneFiles(const std::string &directory) {
  std::vector<FileInfo> allFiles = listDirectory(directory);
  std::vector<FileInfo> sceneFiles;

  for (const auto &file : allFiles) {
    if (!file.isDirectory && file.extension == "scene") {
      sceneFiles.push_back(file);
    }
  }

  return sceneFiles;
}

bool FileBrowser::directoryExists(const std::string &path) {
  return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}

bool FileBrowser::fileExists(const std::string &path) {
  return std::filesystem::exists(path) &&
         std::filesystem::is_regular_file(path);
}

std::string FileBrowser::getParentDirectory(const std::string &path) {
  std::filesystem::path p(path);
  return p.parent_path().string();
}

std::string FileBrowser::getFileName(const std::string &path) {
  std::filesystem::path p(path);
  return p.filename().string();
}

std::string FileBrowser::getFileExtension(const std::string &path) {
  std::filesystem::path p(path);
  std::string ext = p.extension().string();
  if (!ext.empty() && ext[0] == '.') {
    ext = ext.substr(1);
  }
  return ext;
}

std::string FileBrowser::joinPaths(const std::string &path1,
                                   const std::string &path2) {
  std::filesystem::path p1(path1);
  std::filesystem::path p2(path2);
  return (p1 / p2).string();
}

std::string FileBrowser::getCurrentDirectory() {
  try {
    return std::filesystem::current_path().string();
  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "Error getting current directory: " << e.what() << std::endl;
    return "";
  }
}

std::string FileBrowser::getSceneDirectory() {
  std::string currentDir = getCurrentDirectory();
  std::string sceneDir = joinPaths(currentDir, "../resources/scenes");

  // Normalize the path to remove .. references
  try {
    std::filesystem::path normalizedPath = std::filesystem::canonical(sceneDir);
    sceneDir = normalizedPath.string();
  } catch (const std::filesystem::filesystem_error &e) {
    // If canonical fails (e.g., directory doesn't exist), try to normalize
    // manually
    std::filesystem::path path(sceneDir);
    sceneDir = path.lexically_normal().string();
  }

  // Create scenes directory if it doesn't exist
  if (!directoryExists(sceneDir)) {
    try {
      std::filesystem::create_directories(sceneDir);
    } catch (const std::filesystem::filesystem_error &e) {
      std::cerr << "Error creating scenes directory: " << e.what() << std::endl;
    }
  }

  return sceneDir;
}

std::string FileBrowser::formatFileSize(size_t bytes) {
  const char *suffixes[] = {"B", "KB", "MB", "GB", "TB"};
  int suffixIndex = 0;
  double size = static_cast<double>(bytes);

  while (size >= 1024.0 && suffixIndex < 4) {
    size /= 1024.0;
    suffixIndex++;
  }

  char buffer[64];
  if (suffixIndex == 0) {
    sprintf_s(buffer, sizeof(buffer), "%zu %s", bytes, suffixes[suffixIndex]);
  } else {
    sprintf_s(buffer, sizeof(buffer), "%.1f %s", size, suffixes[suffixIndex]);
  }

  return std::string(buffer);
}

// Platform-specific implementations
#ifdef _WIN32
std::string FileBrowser::openFileDialog_Windows(const std::string &title,
                                                const std::string &defaultPath,
                                                const std::string &filter) {
  // Use PowerShell for file dialog to avoid header conflicts
  std::string command =
      "powershell -Command \"Add-Type -AssemblyName System.Windows.Forms; $f = "
      "New-Object System.Windows.Forms.OpenFileDialog; $f.Filter = 'Scene "
      "Files (*.scene)|*.scene|All Files (*.*)|*.*'; $f.Title = '" +
      title + "';";
  if (!defaultPath.empty()) {
    command += " $f.InitialDirectory = '" + defaultPath + "';";
  }
  command += " if ($f.ShowDialog() -eq 'OK') { $f.FileName } else { '' }\"";

  FILE *pipe = _popen(command.c_str(), "r");
  if (!pipe)
    return "";

  char buffer[1024];
  std::string result = "";
  if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    result = buffer;
    // Remove trailing newline/whitespace
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r' ||
                               result.back() == ' ')) {
      result.pop_back();
    }
  }

  _pclose(pipe);
  return result;
}

std::string FileBrowser::saveFileDialog_Windows(
    const std::string &title, const std::string &defaultPath,
    const std::string &defaultFilename, const std::string &filter) {
  // Use PowerShell for save dialog to avoid header conflicts
  std::string command =
      "powershell -Command \"Add-Type -AssemblyName System.Windows.Forms; $f = "
      "New-Object System.Windows.Forms.SaveFileDialog; $f.Filter = 'Scene "
      "Files (*.scene)|*.scene|All Files (*.*)|*.*'; $f.Title = '" +
      title + "';";
  if (!defaultPath.empty()) {
    command += " $f.InitialDirectory = '" + defaultPath + "';";
  }
  if (!defaultFilename.empty()) {
    command += " $f.FileName = '" + defaultFilename + "';";
  }
  command += " $f.DefaultExt = 'scene'; if ($f.ShowDialog() -eq 'OK') { "
             "$f.FileName } else { '' }\"";

  FILE *pipe = _popen(command.c_str(), "r");
  if (!pipe)
    return "";

  char buffer[1024];
  std::string result = "";
  if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    result = buffer;
    // Remove trailing newline/whitespace
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r' ||
                               result.back() == ' ')) {
      result.pop_back();
    }
  }

  _pclose(pipe);
  return result;
}

#elif defined(__linux__)
std::string FileBrowser::openFileDialog_Linux(const std::string &title,
                                              const std::string &defaultPath,
                                              const std::string &filter) {
  // Try zenity first (most common)
  std::string command = "zenity --file-selection --title=\"" + title + "\"";
  if (!defaultPath.empty()) {
    command += " --filename=\"" + defaultPath + "/\"";
  }
  command += " --file-filter=\"Scene files | *.scene\" --file-filter=\"All "
             "files | *\"";

  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe) {
    // Fallback to kdialog
    command = "kdialog --getopenfilename";
    if (!defaultPath.empty()) {
      command += " \"" + defaultPath + "\"";
    }
    command += " \"*.scene|Scene Files\"";

    pipe = popen(command.c_str(), "r");
    if (!pipe) {
      std::cerr << "No file dialog available (zenity or kdialog required)"
                << std::endl;
      return "";
    }
  }

  char buffer[1024];
  std::string result = "";
  if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    result = buffer;
    // Remove trailing newline
    if (!result.empty() && result.back() == '\n') {
      result.pop_back();
    }
  }

  pclose(pipe);
  return result;
}

std::string FileBrowser::saveFileDialog_Linux(
    const std::string &title, const std::string &defaultPath,
    const std::string &defaultFilename, const std::string &filter) {
  std::string command =
      "zenity --file-selection --save --title=\"" + title + "\"";
  if (!defaultPath.empty()) {
    command += " --filename=\"" + defaultPath + "/";
    if (!defaultFilename.empty()) {
      command += defaultFilename;
    }
    command += "\"";
  }
  command += " --file-filter=\"Scene files | *.scene\" --file-filter=\"All "
             "files | *\"";

  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe) {
    // Fallback to kdialog
    command = "kdialog --getsavefilename";
    if (!defaultPath.empty()) {
      command += " \"" + defaultPath + "/";
      if (!defaultFilename.empty()) {
        command += defaultFilename;
      }
      command += "\"";
    }
    command += " \"*.scene|Scene Files\"";

    pipe = popen(command.c_str(), "r");
    if (!pipe) {
      std::cerr << "No file dialog available (zenity or kdialog required)"
                << std::endl;
      return "";
    }
  }

  char buffer[1024];
  std::string result = "";
  if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    result = buffer;
    // Remove trailing newline
    if (!result.empty() && result.back() == '\n') {
      result.pop_back();
    }
  }

  pclose(pipe);
  return result;
}

#elif defined(__APPLE__)
std::string FileBrowser::openFileDialog_macOS(const std::string &title,
                                              const std::string &defaultPath,
                                              const std::string &filter) {
  std::string command =
      R"(osascript -e 'tell application "System Events" to activate' -e 'tell application "System Events" to set thePath to choose file with prompt ")" +
      title + R"(" of type {"scene"} default location POSIX file ")";
  if (!defaultPath.empty()) {
    command += defaultPath;
  } else {
    command += "~/";
  }
  command += R"("' -e 'return POSIX path of thePath')";

  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe) {
    std::cerr << "Error opening file dialog" << std::endl;
    return "";
  }

  char buffer[1024];
  std::string result = "";
  if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    result = buffer;
    // Remove trailing newline
    if (!result.empty() && result.back() == '\n') {
      result.pop_back();
    }
  }

  pclose(pipe);
  return result;
}

std::string FileBrowser::saveFileDialog_macOS(
    const std::string &title, const std::string &defaultPath,
    const std::string &defaultFilename, const std::string &filter) {
  std::string command =
      R"(osascript -e 'tell application "System Events" to activate' -e 'tell application "System Events" to set thePath to choose file name with prompt ")" +
      title + R"(" default name ")";
  if (!defaultFilename.empty()) {
    command += defaultFilename;
  } else {
    command += "scene.scene";
  }
  command += R"(" default location POSIX file ")";
  if (!defaultPath.empty()) {
    command += defaultPath;
  } else {
    command += "~/";
  }
  command += R"("' -e 'return POSIX path of thePath')";

  FILE *pipe = popen(command.c_str(), "r");
  if (!pipe) {
    std::cerr << "Error opening save dialog" << std::endl;
    return "";
  }

  char buffer[1024];
  std::string result = "";
  if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    result = buffer;
    // Remove trailing newline
    if (!result.empty() && result.back() == '\n') {
      result.pop_back();
    }
  }

  pclose(pipe);
  return result;
}
#endif

std::string FileBrowser::parseFilter(const std::string &filter) {
  // Simple filter parsing - can be enhanced later
  return filter;
}