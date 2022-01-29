#pragma once

/**
 * @brief WASAPI-based audio capturing thread.
 *
 * Uses the Windows Audio Session API to capture PCM data from either a loopback device or
 * any pother available input device.
 *
 * The loopback device is always considered as the "first" available device. All other external audio
 * sources come after that.
 */
class AudioCaptureImpl
{

};


