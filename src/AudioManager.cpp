#include "AudioManager.h"
#include <fstream>
#include <iostream>
#include <raudio.h>

AudioManager::AudioManager() : m_initialized(false) {}

AudioManager::~AudioManager() { shutdown(); }

bool AudioManager::initialize() {
  if (m_initialized) {
    std::cout << "AudioManager: Already initialized." << std::endl;
    return true;
  }

  InitAudioDevice();

  if (!IsAudioDeviceReady()) {
    std::cerr << "AudioManager: Failed to initialize audio device."
              << std::endl;
    return false;
  }

  m_initialized = true;
  std::cout << "AudioManager: Successfully initialized audio system."
            << std::endl;

  // Set default master volume
  setMasterVolume(0.5f);

  return true;
}

void AudioManager::shutdown() {
  if (!m_initialized) {
    return;
  }

  // Stop all sounds before cleanup
  stopAllSounds();

  // Unload all sounds
  unloadAllSounds();

  // Close audio device
  CloseAudioDevice();

  m_initialized = false;
  std::cout << "AudioManager: Audio system shut down." << std::endl;
}

bool AudioManager::isInitialized() const { return m_initialized; }

bool AudioManager::loadSound(const std::string &name,
                             const std::string &filepath) {
  if (!m_initialized) {
    std::cerr
        << "AudioManager: Cannot load sound - audio system not initialized."
        << std::endl;
    return false;
  }

  // Check if sound already exists
  if (m_sounds.find(name) != m_sounds.end()) {
    std::cout << "AudioManager: Sound '" << name << "' already loaded."
              << std::endl;
    return true;
  }

  // Check if file exists before attempting to load
  std::ifstream file(filepath.c_str());
  if (!file.good()) {
    std::cerr << "AudioManager: File '" << filepath
              << "' does not exist or is not accessible." << std::endl;
    return false;
  }
  file.close();

  // Load the sound
  Sound loadedSound = LoadSound(filepath.c_str());

  // Check if loading was successful (raudio doesn't provide a direct error
  // check, but we can check if the sound has valid sample count)
  if (loadedSound.sampleCount == 0) {
    std::cerr << "AudioManager: Failed to load sound '" << name << "' from '"
              << filepath
              << "'. File may be corrupted or in unsupported format."
              << std::endl;
    return false;
  }

  // Store the sound
  auto soundPtr = std::make_unique<Sound>(loadedSound);
  m_sounds[name] = std::move(soundPtr);

  std::cout << "AudioManager: Successfully loaded sound '" << name << "' from '"
            << filepath << "'." << std::endl;
  return true;
}

void AudioManager::unloadSound(const std::string &name) {
  auto it = m_sounds.find(name);
  if (it != m_sounds.end()) {
    stopSound(name); // Stop the sound if it's playing
    UnloadSound(*(it->second));
    m_sounds.erase(it);
    std::cout << "AudioManager: Unloaded sound '" << name << "'." << std::endl;
  }
}

void AudioManager::unloadAllSounds() {
  for (auto &pair : m_sounds) {
    UnloadSound(*(pair.second));
  }
  m_sounds.clear();
  std::cout << "AudioManager: Unloaded all sounds." << std::endl;
}

void AudioManager::playSound(const std::string &name) {
  auto it = m_sounds.find(name);
  if (it != m_sounds.end()) {
    PlaySound(*(it->second));
  } else {
    std::cerr << "AudioManager: Sound '" << name << "' not found." << std::endl;
  }
}

void AudioManager::playSoundMulti(const std::string &name) {
  auto it = m_sounds.find(name);
  if (it != m_sounds.end()) {
    PlaySoundMulti(*(it->second));
  } else {
    std::cerr << "AudioManager: Sound '" << name << "' not found." << std::endl;
  }
}

void AudioManager::stopSound(const std::string &name) {
  auto it = m_sounds.find(name);
  if (it != m_sounds.end()) {
    StopSound(*(it->second));
  } else {
    std::cerr << "AudioManager: Sound '" << name << "' not found." << std::endl;
  }
}

void AudioManager::pauseSound(const std::string &name) {
  auto it = m_sounds.find(name);
  if (it != m_sounds.end()) {
    PauseSound(*(it->second));
  } else {
    std::cerr << "AudioManager: Sound '" << name << "' not found." << std::endl;
  }
}

void AudioManager::resumeSound(const std::string &name) {
  auto it = m_sounds.find(name);
  if (it != m_sounds.end()) {
    ResumeSound(*(it->second));
  } else {
    std::cerr << "AudioManager: Sound '" << name << "' not found." << std::endl;
  }
}

void AudioManager::setSoundVolume(const std::string &name, float volume) {
  auto it = m_sounds.find(name);
  if (it != m_sounds.end()) {
    SetSoundVolume(*(it->second), volume);
  } else {
    std::cerr << "AudioManager: Sound '" << name << "' not found." << std::endl;
  }
}

void AudioManager::setSoundPitch(const std::string &name, float pitch) {
  auto it = m_sounds.find(name);
  if (it != m_sounds.end()) {
    SetSoundPitch(*(it->second), pitch);
  } else {
    std::cerr << "AudioManager: Sound '" << name << "' not found." << std::endl;
  }
}

bool AudioManager::isSoundPlaying(const std::string &name) {
  auto it = m_sounds.find(name);
  if (it != m_sounds.end()) {
    return IsSoundPlaying(*(it->second));
  }
  return false;
}

void AudioManager::setMasterVolume(float volume) {
  if (m_initialized) {
    SetMasterVolume(volume);
  }
}

void AudioManager::stopAllSounds() {
  if (m_initialized) {
    StopSoundMulti(); // Stops all sounds playing in multichannel buffer pool

    // Also stop individual sounds
    for (auto &pair : m_sounds) {
      StopSound(*(pair.second));
    }
  }
}

int AudioManager::getSoundsPlaying() const {
  if (m_initialized) {
    return GetSoundsPlaying();
  }
  return 0;
}