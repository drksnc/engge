#pragma once
#include <array>
#include <memory>
#include <vector>
#include "SoundCategory.hpp"
#include "SoundId.hpp"
#include "SoundDefinition.hpp"

namespace ng {
class Entity;
class Engine;
class SoundDefinition;
class SoundId;

class SoundManager {
public:
  SoundManager();

  void setEngine(Engine *pEngine) { _pEngine = pEngine; }
  [[nodiscard]] Engine *getEngine() const { return _pEngine; }

  SoundDefinition *defineSound(const std::string &name);
  SoundId *playSound(SoundDefinition *pSoundDefinition, int loopTimes = 1, int id = 0);
  SoundId *playTalkSound(SoundDefinition *pSoundDefinition, int loopTimes = 1, int id = 0);
  SoundId *playMusic(SoundDefinition *pSoundDefinition, int loopTimes = 1);

  void pauseAllSounds();
  void resumeAllSounds();

  void stopAllSounds();
  void stopSound(SoundId *pSound);
  void stopSound(const SoundDefinition *pSoundDefinition);

  void setMasterVolume(float volume) { _masterVolume = std::clamp(volume, 0.f, 1.f); }
  [[nodiscard]] float getMasterVolume() const { return _masterVolume; }
  void setSoundVolume(float volume) { _soundVolume = std::clamp(volume, 0.f, 1.f); }
  [[nodiscard]] float getSoundVolume() const { return _soundVolume; }
  void setMusicVolume(float volume) { _musicVolume = std::clamp(volume, 0.f, 1.f); }
  [[nodiscard]] float getMusicVolume() const { return _musicVolume; }
  void setTalkVolume(float volume) { _talkVolume = std::clamp(volume, 0.f, 1.f); }
  [[nodiscard]] float getTalkVolume() const { return _talkVolume; }
  void setVolume(const SoundDefinition *pSoundDefinition, float volume);

  SoundId *getSound(size_t index);
  std::vector<std::unique_ptr<SoundDefinition>> &getSoundDefinitions() { return _sounds; }
  std::array<std::unique_ptr<SoundId>, 32> &getSounds() { return _soundIds; }

  [[nodiscard]] size_t getSize() const { return _soundIds.size(); }

  void update(const ngf::TimeSpan &elapsed);

private:
  int getSlotIndex();
  SoundId *play(SoundDefinition *pSoundDefinition,
                SoundCategory category,
                int loopTimes = 1,
                int id = 0);

private:
  std::vector<std::unique_ptr<SoundDefinition>> _sounds;
  std::array<std::unique_ptr<SoundId>, 32> _soundIds;
  Engine *_pEngine{nullptr};
  float _masterVolume{1};
  float _soundVolume{1};
  float _musicVolume{1};
  float _talkVolume{1};
};
} // namespace ng