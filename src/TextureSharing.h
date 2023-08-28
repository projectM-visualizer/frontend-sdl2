#pragma once

#include <SDL2/SDL.h>

#include <Poco/Logger.h>

#include <glad/glad.h>

#include <Syphon/Syphon.h>
/**
 * @brief Class to help with texture sharing to other apps via Syphon/Spout.
 */
class TextureSharing
{
public:
    /**
     * @brief Performs initialization of texture sharing.
     */
    void initialize(int width, int height);

    /**
     * @brief Publishes current texture.
     */
    void publish();

protected:
    void createFramebuffer(int width, int height);

    Poco::Logger& _logger{Poco::Logger::get("TextureSharing")}; //!< The class logger.

    GLuint _shareFamebuffer, _shareTexture;
    int _width, _height;

    SyphonOpenGLServer* _syphonServer = NULL;
};
