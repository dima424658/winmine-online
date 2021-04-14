#include <SDL2/SDL.h>

#include "sound.hpp"

#include <thread>

int main()
{
    Sound sound;

    sound.PlayTune(Tune::WinGame);
    sound.PlayTune(Tune::WinGame);

    std::this_thread::sleep_for(std::chrono::seconds(10000));


    return 0;
}