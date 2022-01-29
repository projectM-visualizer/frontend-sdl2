#pragma once

#include <cstdint>

/**
 * @brief Limits FPS by adding a delay if necessary. Also keeps track of actual FPS.
 */
class FPSLimiter
{
public:
    /**
     * @brief Sets the target frames per second value.
     * @param fps The targeted frames per second. Set to 0 for unlimited FPS.
     */
    void TargetFPS(int fps);

    /**
     * @brief Calculates the current real FPS.
     *
     * Frames with zero time are not considered.
     *
     * @return The frames per second value as an average over the last ten frames. Zero if all frame times are zero.
     */
    float FPS() const;

    /**
     * @brief Marks the start of a new frame.
     *
     * Should be the first call in the render loop.
     */
    void StartFrame();

    /**
     * @brief Marks the end of a frame.
     *
     * Will pause if required to lower FPS to target value. Also records the last frame time for FPS calculation.
     */
    void EndFrame();

protected:

    uint32_t _lastTickCount{ 0 }; //!< Last SDL tick count, when a new frame was started.
    uint32_t _targetFrameTime{ 0 }; //!< Targeted time per frame in milliseconds.
    uint32_t _lastFrameTimes[10]{}; //!< Actual tick time of the last ten frames, including limiting delay.
    int _nextFrameTimesOffset{ 0 }; //!< Next offset to overwrite the _lastFrameTimes ring buffer.

};


