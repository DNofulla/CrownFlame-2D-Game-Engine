#pragma once

#include <memory>
#include <string>
#include <unordered_map>

// Forward declarations to avoid including raudio.h in header
struct Sound;

class AudioManager {
public:
  AudioManager();
  ~AudioManager();

  // Audio system lifecycle
  bool initialize();
  void shutdown();
  bool isInitialized() const;

  // Sound loading and management
  bool loadSound(const std::string &name, const std::string &filepath);
  void unloadSound(const std::string &name);
  void unloadAllSounds();

  // Sound playback
  void playSound(const std::string &name);
  void playSoundMulti(const std::string &name); // For sounds that can overlap
  void stopSound(const std::string &name);
  void pauseSound(const std::string &name);
  void resumeSound(const std::string &name);

  // Sound properties
  void setSoundVolume(const std::string &name, float volume);
  void setSoundPitch(const std::string &name, float pitch);
  bool isSoundPlaying(const std::string &name);

  // Global audio settings
  void setMasterVolume(float volume);
  void stopAllSounds();
  int getSoundsPlaying() const;

private:
  bool m_initialized;
  std::unordered_map<std::string, std::unique_ptr<Sound>> m_sounds;

  // Disable copy constructor and assignment operator
  AudioManager(const AudioManager &) = delete;
  AudioManager &operator=(const AudioManager &) = delete;
};