#ifndef DOUBLE_FBO_SHADING
#define DOUBLE_FBO_SHADING

#include "utils.h"

typedef struct {
    GLuint fboId;
    GLuint textureId;
} FBOData;

typedef struct {
    FBOData input;
    FBOData output;
} DoubleFBO;

// Returns 0 upon successful creation of DoubleFBO
int double_fbo_create(DoubleFBO *dfbo, int width, int height)
{
    assert(dfbo && "Expected non-null argument");
    
    GLuint fbos[2];
    CHKERRGL(glGenFramebuffers(2, fbos));
    
    CHKERRGL(glBindFramebuffer(GL_READ_FRAMEBUFFER, fbos[0]));
    CHKERRGL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbos[1]));
    
    // Generate color attachment texture for fbos:
    GLuint fboTextures[2];
    CHKERRGL(glGenTextures(2, fboTextures));
    
    // Prepare initial state texture contents in RGBA32F format
    int nChannels = 4;  // RGBA
    GLfloat *initialState = NULL;
    CHKALLOC(initialState = (GLfloat *)malloc(sizeof(GLfloat)*nChannels*width*height));
    
    for (int ih = 0; ih < height; ++ih)
    {
        for (int iw = 0; iw < width; ++iw)
        {
            const int ip = ih * width + iw;
            
            // Map to [0, 1]^2 range
            const GLfloat u = (iw + 0.5f) / width;
            const GLfloat v = (ih + 0.5f) / height;
            
            // Map to [-pi, pi]^2 range (initial angle conditions)
            initialState[nChannels * ip + 0] = M_PI * (2.0f * u - 1.0f);  // R
            initialState[nChannels * ip + 1] = M_PI * (2.0f * v - 1.0f);  // G
            initialState[nChannels * ip + 2] = 0.0f;  // B
            initialState[nChannels * ip + 3] = 0.0f;  // A
        }
    }
    
    // Input texture for initial state
    CHKERRGL(glBindTexture(GL_TEXTURE_2D, fboTextures[0]));
    CHKERRGL(glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA32F,
        width,
        height,
        0,
        GL_RGBA,
        GL_FLOAT,
        (void *)initialState)
    );
    CHKERRGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHKERRGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    
    free(initialState);  // no longer needed, since texture data is copied
    
    CHKERRGL(glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTextures[0], 0));
    
    // Target output texture for next state
    CHKERRGL(glBindTexture(GL_TEXTURE_2D, fboTextures[1]));
    CHKERRGL(glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA32F,
        width,
        height,
        0,
        GL_RGBA,
        GL_FLOAT,
        NULL)
    );
    CHKERRGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHKERRGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    
    CHKERRGL(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTextures[1], 0));
    
    CHKERRGL(glBindTexture(GL_TEXTURE_2D, 0));  // unbind
    
    // Check framebuffers for completeness
    if (glCheckFramebufferStatus(GL_READ_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        return -1;
    }
    
    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        return -1;
    }
    
    CHKERRGL(glBindFramebuffer(GL_FRAMEBUFFER, 0));  // unbind
    
    dfbo->input  = (FBOData){ .fboId = fbos[0], .textureId = fboTextures[0] };
    dfbo->output = (FBOData){ .fboId = fbos[1], .textureId = fboTextures[1] };
    
    return 0;
}

void double_fbo_swap(DoubleFBO *dfbo)
{
    assert(dfbo && "Expected non-null argument");
    
    GLuint temp;
    
    // Swap ids of FBOs
    temp = dfbo->input.fboId;
    dfbo->input.fboId = dfbo->output.fboId;
    dfbo->output.fboId = temp;
    
    // Swap texture ids of FBOs
    temp = dfbo->input.textureId;
    dfbo->input.textureId = dfbo->output.textureId;
    dfbo->output.textureId = temp;
}

void double_fbo_destroy(DoubleFBO *dfbo)
{
    assert(dfbo && "Expected non-null argument");
    
    // Not sure if textures have to be deleted separately from framebuffers...
    CHKERRGL(glBindFramebuffer(GL_READ_FRAMEBUFFER, dfbo->input.fboId));
    CHKERRGL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dfbo->output.fboId));
    
    // Detach textures from FBOs
    CHKERRGL(glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0));
    CHKERRGL(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0));
    
    CHKERRGL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    
    GLuint textures[2] = {dfbo->input.textureId, dfbo->output.textureId};
    CHKERRGL(glDeleteTextures(2, textures));
    
    GLuint fbos[2] = {dfbo->input.fboId, dfbo->output.fboId};
    CHKERRGL(glDeleteFramebuffers(2, fbos));
}

#endif /* DOUBLE_FBO_SHADING */
