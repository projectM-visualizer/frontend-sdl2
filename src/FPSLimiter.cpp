#include "FPSLimiter.h"

#include <SDL2/SDL.h>

void FPSLimiter::TargetFPS(int fps)
{
    if (fps)
    {
        _targetFrameTime = 1000 / fps;
    }
    else
    {
        _targetFrameTime = 0;
    }
}

float FPSLimiter::FPS() const
{
    uint32_t frameTimeSum{ 0 };
    uint32_t frameTimeCount{ 0 };

    for (auto _lastFrameTime : _lastFrameTimes)
    {
        if (_lastFrameTime > 0)
        {
            frameTimeCount++;
            frameTimeSum += _lastFrameTime;
        }
    }

    if (frameTimeCount == 0)
    {
        return 0.0f;
    }

   return 1000.0f / (static_cast<float>(frameTimeSum) / static_cast<float>(frameTimeCount));
}

void FPSLimiter::StartFrame()
{
    _lastTickCount = SDL_GetTicks();
}

void FPSLimiter::EndFrame()
{
    uint32_t frameTime = SDL_GetTicks() - _lastTickCount;

    if (_targetFrameTime && frameTime < _targetFrameTime)
    {
        SDL_Delay(_targetFrameTime - frameTime);
        frameTime = SDL_GetTicks() - _lastTickCount;
    }

    _lastFrameTimes[_nextFrameTimesOffset] = frameTime;
    _nextFrameTimesOffset = (_nextFrameTimesOffset + 1) % 10;
}
