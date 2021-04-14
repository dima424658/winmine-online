#include "sound.hpp"
#include "defines.hpp"

Sound::Sound() : m_tick(g_tune_tick), m_winGame(g_tune_won), m_loseGame(g_tune_lost)
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0)
        throw std::runtime_error("Failed to init SDL Audio.");

    auto want = m_tick.GetSpec();

    want.callback = Sound::Callback;
    want.userdata = &m_queue;

    if (m_device = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE) == 0)
        throw std::runtime_error("Failed to open audio device.");
}

Sound::~Sound()
{
    if (m_device)
    {
        SDL_CloseAudioDevice(m_device);
        m_device = 0;
    }
}

void Sound::PlayTune(Tune tune) noexcept
{
    SDL_LockAudioDevice(m_device);

    switch (tune)
    {
    case Tune::Tick:
        m_queue.emplace_back(m_tick, 0);
        break;
    case Tune::WinGame:
        m_queue.emplace_back(m_winGame, 0);
        break;
    case Tune::LoseGame:
        m_queue.emplace_back(m_loseGame, 0);
        break;
    }

    SDL_UnlockAudioDevice(m_device);
}

void Sound::Callback(void *userdata, Uint8 *stream, int len) noexcept
{
    SDL_memset(stream, 0, len);
    auto &list = *reinterpret_cast<decltype(Sound::m_queue) *>(userdata);

    if (list.size() > 0)
    {
        const auto &wav = list.front().first;
        auto &seek = list.front().second;

        if (seek < wav.GetLength())
        {
            auto tempLength = static_cast<Uint32>(len) > (wav.GetLength() - seek) ? (wav.GetLength() - seek) : static_cast<Uint32>(len);

            SDL_MixAudioFormat(stream, wav.GetBuffer() + seek, wav.GetSpec().format, tempLength, SDL_MIX_MAXVOLUME);

            seek += tempLength;
        }
        else
            list.pop_front();
    }
}

Wav::Wav(std::string_view path)
{
    if (SDL_LoadWAV(path.data(), &m_spec, &m_buffer, &m_length) == nullptr)
        throw std::runtime_error("Failed to open wav file.");
}

Wav::~Wav()
{
    if (m_buffer)
    {
        SDL_FreeWAV(m_buffer);
        m_buffer = nullptr;
    }
}

const SDL_AudioSpec &Wav::GetSpec() const noexcept
{
    return m_spec;
}

const Uint8 *Wav::GetBuffer() const noexcept
{
    return m_buffer;
}

const Uint32 Wav::GetLength() const noexcept
{
    return m_length;
}