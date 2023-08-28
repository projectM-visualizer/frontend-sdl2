#include "TextureSharing.h"

#include <Poco/Util/Application.h>

void TextureSharing::createFramebuffer(int width, int height)
{
    glGenFramebuffers(1, &_shareFamebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _shareFamebuffer);

    glGenTextures(1, &_shareTexture);
    glBindTexture(GL_TEXTURE_2D, _shareTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _shareTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        poco_error_f1(_logger, "Failed to initialize texture sharing framebuffer: %s", std::string(SDL_GetError()));
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TextureSharing::initialize(int width, int height)
{
    // Syphon setup
    CGLContextObj cgl_ctx = CGLGetCurrentContext();
    _syphonServer = [[SyphonOpenGLServer alloc] initWithName:nil context:cgl_ctx options:nil];

    _width = width;
    _height = height;

    createFramebuffer(_width, _height);
}

void TextureSharing::publish()
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _shareFamebuffer);

    glBlitFramebuffer(0, 0, _width, _height, 0, 0, _width, _height, GL_COLOR_BUFFER_BIT, GL_LINEAR);

    [_syphonServer publishFrameTexture:_shareTexture textureTarget:GL_TEXTURE_2D imageRegion:NSMakeRect(0, 0, _width, _height) textureDimensions:NSMakeSize(_width, _height) flipped:NO];
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}