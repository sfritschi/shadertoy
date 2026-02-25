#ifndef UTILS_H
#define UTILS_H
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#define M_PI 3.1415926535897932384626433832795

#define CHKALLOC(ptr)                            \
do                                               \
{                                                \
    if (!(ptr)) {                                \
        fprintf(stderr, "Buy more RAM, lol\n");  \
        exit(EXIT_FAILURE);                      \
    }                                            \
}                                                \
while(0)

// TODO: Add cleanup call instead of exit
#define CHKERRGL(call)                                                 \
do                                                                     \
{                                                                      \
    call;                                                              \
    const GLenum err = glGetError();                                   \
    if (err != GL_NO_ERROR) {                                          \
        fprintf(stderr, "GL error in %s:%d, %s\n", __FILE__, __LINE__, \
            getGLErrorString(err));                                    \
        exit(EXIT_FAILURE);                                            \
    }                                                                  \
}                                                                      \
while (0)

const char *getGLErrorString(GLenum error)
{
    switch (error) {
        case GL_NO_ERROR:            return "No error";
        case GL_INVALID_ENUM:        return "Invalid enum";
        case GL_INVALID_VALUE:       return "Invalid value";
        case GL_INVALID_OPERATION:   return "Invalid operation";
        case GL_STACK_OVERFLOW:      return "Stack overflow";
        case GL_STACK_UNDERFLOW:     return "Stack underflow";
        case GL_OUT_OF_MEMORY:       return "Out of memory";
        case GL_TABLE_TOO_LARGE:     return "Table too large";
        default:                     return "Unknown GL error";
    }
}

void glfwError(int error, const char *description)
{
    fprintf(stderr, "Error: %d, %s\n", error, description);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    (void)scancode;  // unused
    (void)mods;      // unused
    
    // Set window close flag if either 'Q' or 'Esc' is pressed
    if ((key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

GLFWwindow *createGLFWwindow(int windowWidth, int windowHeight)
{
    if (!glfwInit())
    {
        return NULL;
    }
    glfwSetErrorCallback(glfwError);
    // Minimum required version of OpenGL/GLSL is 4.2.
    // This is required for C-style array initializers used in GLSL shaders
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);  // no resizing of window for now...
    
    GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "Shadertoy", NULL, NULL);
    if (!window)
    {
        return NULL;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    
    const GLenum glewError = glewInit();
    if (glewError != GLEW_OK)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return NULL;
    }
    glfwSwapInterval(1);  // synchronize to display refresh rate
    
    return window;
}

bool createGLShader(const GLenum shaderType, const char *shaderFile, GLuint *shader)
{
    assert(shader && "Expected non-NULL argument");
    
    *shader = 0;
    
    FILE *fp = fopen(shaderFile, "r");
    if (!fp)
    {
        fprintf(stderr, "Failed to open shader file: '%s'. "
            "Reason: %s\n", shaderFile, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    fseek(fp, 0L, SEEK_END);
    const GLint shaderSize = ftell(fp);
    rewind(fp);
    
    GLchar *shaderSrc = NULL;
    CHKALLOC(shaderSrc = (GLchar *) malloc(shaderSize * sizeof(GLchar)));
    fread(shaderSrc, sizeof(GLchar), shaderSize, fp);
    // File no longer needed
    fclose(fp);
    
    bool usesFBO = false;
    if (shaderType == GL_FRAGMENT_SHADER)
    {
        // Check if fragment shader uses FBOs by looking for 'sampler2D' occurrence in src
        if (strstr(shaderSrc, "sampler2D"))
        {
            usesFBO = true;
        }
    }
    
    CHKERRGL(*shader = glCreateShader(shaderType));
    CHKERRGL(glShaderSource(*shader, 1, (const GLchar **)&shaderSrc, &shaderSize));
    free(shaderSrc);  // Note: glShaderSource copies source code
    CHKERRGL(glCompileShader(*shader));
    
    // Make sure shader was compiled successfully
    GLint compileStatus = GL_FALSE;
    CHKERRGL(glGetShaderiv(*shader, GL_COMPILE_STATUS, &compileStatus));
    if (compileStatus != GL_TRUE)
    {
        // Obtain and print compiler error message
        GLint infoLogLength = 0;
        CHKERRGL(glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &infoLogLength));
        
        GLchar *infoLog = NULL;
        CHKALLOC(infoLog = (GLchar *) malloc(infoLogLength * sizeof(GLchar)));
        CHKERRGL(glGetShaderInfoLog(*shader, infoLogLength, NULL, infoLog));
        
        fprintf(stderr, "Shader Compilation Error: %s\n", infoLog);
        
        // Cleanup
        free(infoLog);
        CHKERRGL(glDeleteShader(*shader));
        
        exit(EXIT_FAILURE);
    }
    
    return usesFBO;
}

bool createGLShaderProgram(const char *fshader, GLuint *shaderProgram)
{
    assert(shaderProgram && "Expected non-NULL argument");
    
    *shaderProgram = 0;
    
    GLuint fragShader, vertShader;
    bool usesFBO = createGLShader(GL_FRAGMENT_SHADER, fshader, &fragShader);
    createGLShader(GL_VERTEX_SHADER, "shaders/shader.vert", &vertShader);
    
    CHKERRGL(*shaderProgram = glCreateProgram());
    CHKERRGL(glAttachShader(*shaderProgram, fragShader));
    CHKERRGL(glAttachShader(*shaderProgram, vertShader));
    CHKERRGL(glLinkProgram(*shaderProgram));
    
    GLint linkStatus = GL_FALSE;
    CHKERRGL(glGetProgramiv(*shaderProgram, GL_LINK_STATUS, &linkStatus));
    if (linkStatus != GL_TRUE)
    {
        // Obtain and print linker error message
        GLint infoLogLength = 0;
        CHKERRGL(glGetProgramiv(*shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength));
        
        GLchar *infoLog = NULL;
        CHKALLOC(infoLog = (GLchar *) malloc(infoLogLength * sizeof(GLchar)));
        CHKERRGL(glGetProgramInfoLog(*shaderProgram, infoLogLength, NULL, infoLog));
        
        fprintf(stderr, "Shader Linking Error: %s\n", infoLog);
        
        // Cleanup
        free(infoLog);
        CHKERRGL(glDetachShader(*shaderProgram, fragShader));
        CHKERRGL(glDetachShader(*shaderProgram, vertShader));
        CHKERRGL(glDeleteShader(fragShader));
        CHKERRGL(glDeleteShader(vertShader));
        CHKERRGL(glDeleteProgram(*shaderProgram));
        
        exit(EXIT_FAILURE);
    }
    // Cleanup
    CHKERRGL(glDetachShader(*shaderProgram, fragShader));
    CHKERRGL(glDetachShader(*shaderProgram, vertShader));
    CHKERRGL(glDeleteShader(fragShader));
    CHKERRGL(glDeleteShader(vertShader));
    
    return usesFBO;
}

#endif /* UTILS_H */
