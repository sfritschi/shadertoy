#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "double_fbo_shading.h"

// Render a screen-covering quad colored by output of fragment shader
void render(const GLuint shaderProgram, const GLfloat width, 
    const GLfloat height, const GLfloat currentTime)
{
    CHKERRGL(glUseProgram(shaderProgram));
        
    GLint viewPortSizeLocation;
    CHKERRGL(viewPortSizeLocation = glGetUniformLocation(shaderProgram, "viewPortSize"));
    CHKERRGL(glUniform2f(viewPortSizeLocation, width, height));
    GLint currentTimeLocation;
    CHKERRGL(currentTimeLocation = glGetUniformLocation(shaderProgram, "currentTime"));
    CHKERRGL(glUniform1fv(currentTimeLocation, 1, &currentTime));
    
    CHKERRGL(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));  // draw quad
}

// Render iterative output of fragment shader to texture bound to output FBO,
// while reading previous iteration's output from texture bound to input FBO 
void renderToFBO(const GLuint shaderProgram, const GLfloat width, 
    const GLfloat height, const GLfloat deltaTime, const DoubleFBO *dfbo)
{
    assert(dfbo && "Expected non-NULL argument");
    
    CHKERRGL(glUseProgram(shaderProgram));
    
    CHKERRGL(glBindFramebuffer(GL_READ_FRAMEBUFFER, dfbo->input.fboId));
    CHKERRGL(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dfbo->output.fboId));
    CHKERRGL(glBindTexture(GL_TEXTURE_2D, dfbo->input.textureId));
        
    GLint viewPortSizeLocation;
    CHKERRGL(viewPortSizeLocation = glGetUniformLocation(shaderProgram, "viewPortSize"));
    CHKERRGL(glUniform2f(viewPortSizeLocation, width, height));
    GLint deltaTimeLocation;
    CHKERRGL(deltaTimeLocation = glGetUniformLocation(shaderProgram, "deltaTime"));
    CHKERRGL(glUniform1fv(deltaTimeLocation, 1, &deltaTime));
    
    CHKERRGL(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));  // draw quad
    
    CHKERRGL(glBindTexture(GL_TEXTURE_2D, 0));
    CHKERRGL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

// Render current iteration's output to screen mapped to adequate color range
void renderToScreen(const GLuint shaderProgram, const GLfloat width,
    const GLfloat height, const DoubleFBO *dfbo)
{
    assert(dfbo && "Expected non-NULL argument");
    
    CHKERRGL(glUseProgram(shaderProgram));
        
    CHKERRGL(glBindTexture(GL_TEXTURE_2D, dfbo->output.textureId));
    
    GLint viewPortSizeLocation;
    CHKERRGL(viewPortSizeLocation = glGetUniformLocation(shaderProgram, "viewPortSize"));
    CHKERRGL(glUniform2f(viewPortSizeLocation, width, height));
    
    CHKERRGL(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));  // draw quad
    
    CHKERRGL(glBindTexture(GL_TEXTURE_2D, 0));
}

int main(int argc, char *argv[])
{
    const char *fshader = "shaders/roots.frag";  // default
    if (argc > 1)
    {
        // Use first given argument as fragment shader file location
        fshader = argv[1];
    }
    
    const int windowWidth  = 800;
    const int windowHeight = 800;
    GLFWwindow *window = createGLFWwindow(windowWidth, windowHeight);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        return EXIT_FAILURE;
    }
    
    // Setup basic OpenGL options
    CHKERRGL(glClearColor(0.0, 0.0, 0.0, 1.0));
    
    // Create shader program and attach it to current context
    GLuint shaderProgram = 0, basicProgram = 0;
    bool usesFBO = createGLShaderProgram(fshader, &shaderProgram);
    
    if (usesFBO)
        createGLShaderProgram("shaders/basic.frag", &basicProgram);
    
    // Note: OpenGL versions 3.3+ require non-zero VAO to be bound.
    //       Otherwise, Invalid Operation is triggered by glDrawArrays(...) 
    GLuint vao;
    CHKERRGL(glGenVertexArrays(1, &vao));
    CHKERRGL(glBindVertexArray(vao));
    
    DoubleFBO dfbo = {0};
    if (usesFBO && double_fbo_create(&dfbo, windowWidth, windowHeight) != 0)
    {
        fprintf(stderr, "Failed to create FBOs needed for iterative shading\n");
        goto cleanup;
    }
    
    GLdouble previousTime = 0.0, currentTime = 0.0;
    glfwSetTime(currentTime);
    
    // Rendering loop
    while (!glfwWindowShouldClose(window))
    {
        GLint width, height;
        glfwGetFramebufferSize(window, &width, &height);
        
        CHKERRGL(glViewport(0, 0, width, height));
        CHKERRGL(glClear(GL_COLOR_BUFFER_BIT));
        
        previousTime = currentTime;
        currentTime = glfwGetTime();
        
        const GLdouble deltaTime = currentTime - previousTime;
        
        if (usesFBO)
        {
            renderToFBO(shaderProgram, width, height, deltaTime, &dfbo);
            renderToScreen(basicProgram, width, height, &dfbo);
        }
        else
        {
            render(shaderProgram, width, height, currentTime);
        }
        
        // Swap FBOs and respective textures
        if (usesFBO)
            double_fbo_swap(&dfbo);
        
        // Swap buffers from double buffering and display contents
        glfwSwapBuffers(window);
        glfwPollEvents();  // needed for window/input callbacks
    }
    
    // Cleanup
cleanup:
    if (usesFBO)
        double_fbo_destroy(&dfbo);
    
    CHKERRGL(glBindVertexArray(0));
    CHKERRGL(glDeleteVertexArrays(1, &vao));
    CHKERRGL(glUseProgram(0));
    CHKERRGL(glDeleteProgram(shaderProgram));
    
    if (usesFBO)
        CHKERRGL(glDeleteProgram(basicProgram));
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return EXIT_SUCCESS;
}
