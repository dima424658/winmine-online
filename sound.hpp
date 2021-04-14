#pragma once

#include <list>
#include <stdexcept>

#include <SDL2/SDL.h>

enum class Tune
{
    Tick,
    WinGame,
    LoseGame
};

class Wav
{
public:
    Wav(std::string_view path);

    Wav(const Wav &) = delete;
    Wav &operator=(const Wav &) = delete;
    Wav(Wav &&) = delete;
    Wav &operator=(Wav &&) = delete;
    ~Wav();

    const SDL_AudioSpec &GetSpec() const noexcept;
    const Uint8 *GetBuffer() const noexcept;
    const Uint32 GetLength() const noexcept;

private:
    SDL_AudioSpec m_spec;
    Uint8 *m_buffer;
    Uint32 m_length;
};

class Sound
{
public:
    Sound();

    Sound(const Sound &) = delete;
    Sound &operator=(const Sound &) = delete;
    Sound(Sound &&) = delete;
    Sound &operator=(Sound &&) = delete;
    ~Sound();

    void PlayTune(Tune) noexcept;

private:
    static void Callback(void *userdata, Uint8 *stream, int len) noexcept;

private:
    SDL_AudioDeviceID m_device;
    std::list<std::pair<Wav &, Uint32>> m_queue;
    Wav m_tick, m_winGame, m_loseGame;
};