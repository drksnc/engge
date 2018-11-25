#include "SoundDefinition.h"

namespace gg
{

SoundDefinition::SoundDefinition(const std::string &path)
    : _path(path), _isLoaded(false)
{
}

void SoundDefinition::load()
{
    if (_isLoaded)
        return;
    _isLoaded = _buffer.loadFromFile(_path);
    if (!_isLoaded)
    {
        std::cerr << "Can't load the sound" << _path << std::endl;
    }
}

SoundId::SoundId(SoundDefinition &soundDefinition)
    : _soundDefinition(soundDefinition)
{
}

SoundId::~SoundId()
{
    stop();
}

void SoundId::play(bool loop)
{
    _soundDefinition.load();
    _sound.setBuffer(_soundDefinition._buffer);
    _sound.setLoop(false);
    _sound.play();
}

void SoundId::setVolume(float volume)
{
    std::cout << "setVolume(" << volume << ")" << std::endl;
    _sound.setVolume(volume);
}

float SoundId::getVolume() const
{
    return _sound.getVolume();
}

void SoundId::stop()
{
    _sound.stop();
}

} // namespace gg