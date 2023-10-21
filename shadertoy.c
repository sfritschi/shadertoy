#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

GLuint createGLShader(const GLenum shaderType, const char *shaderFile)
{
    GLuint shader = 0;
    
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
    
    CHKERRGL(shader = glCreateShader(shaderType));
    CHKERRGL(glShaderSource(shader, 1, (const GLchar **)&shaderSrc, &shaderSize));
    free(shaderSrc);  // Note: glShaderSource copies source code
    CHKERRGL(glCompileShader(shader));
    
    // Make sure shader was compiled successfully
    GLint compileStatus = GL_FALSE;
    CHKERRGL(glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus));
    if (compileStatus != GL_TRUE)
    {
        // Obtain and print compiler error message
        GLint infoLogLength = 0;
        CHKERRGL(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength));
        
        GLchar *infoLog = NULL;
        CHKALLOC(infoLog = (GLchar *) malloc(infoLogLength * sizeof(GLchar)));
        CHKERRGL(glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog));
        
        fprintf(stderr, "Shader Compilation Error: %s\n", infoLog);
        
        // Cleanup
        free(infoLog);
        CHKERRGL(glDeleteShader(shader));
        
        exit(EXIT_FAILURE);
    }
    
    return shader;
}

GLuint createGLShaderProgram(const char *fshader)
{
    GLuint shaderProgram = 0;
    
    const GLuint fragShader = createGLShader(GL_FRAGMENT_SHADER, fshader);
    const GLuint vertShader = createGLShader(GL_VERTEX_SHADER, "shaders/shader.vert");
    
    CHKERRGL(shaderProgram = glCreateProgram());
    CHKERRGL(glAttachShader(shaderProgram, fragShader));
    CHKERRGL(glAttachShader(shaderProgram, vertShader));
    CHKERRGL(glLinkProgram(shaderProgram));
    
    GLint linkStatus = GL_FALSE;
    CHKERRGL(glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus));
    if (linkStatus != GL_TRUE)
    {
        // Obtain and print linker error message
        GLint infoLogLength = 0;
        CHKERRGL(glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &infoLogLength));
        
        GLchar *infoLog = NULL;
        CHKALLOC(infoLog = (GLchar *) malloc(infoLogLength * sizeof(GLchar)));
        CHKERRGL(glGetProgramInfoLog(shaderProgram, infoLogLength, NULL, infoLog));
        
        fprintf(stderr, "Shader Linking Error: %s\n", infoLog);
        
        // Cleanup
        free(infoLog);
        CHKERRGL(glDetachShader(shaderProgram, fragShader));
        CHKERRGL(glDetachShader(shaderProgram, vertShader));
        CHKERRGL(glDeleteShader(fragShader));
        CHKERRGL(glDeleteShader(vertShader));
        CHKERRGL(glDeleteProgram(shaderProgram));
        
        exit(EXIT_FAILURE);
    }
    // Cleanup
    CHKERRGL(glDetachShader(shaderProgram, fragShader));
    CHKERRGL(glDetachShader(shaderProgram, vertShader));
    CHKERRGL(glDeleteShader(fragShader));
    CHKERRGL(glDeleteShader(vertShader));
    
    return shaderProgram;
}

void render(const GLuint shaderProgram, const GLfloat width, 
    const GLfloat height, const GLfloat currentTime)
{
    GLint viewPortSizeLocation;
    CHKERRGL(viewPortSizeLocation = glGetUniformLocation(shaderProgram, "viewPortSize"));
    CHKERRGL(glUniform2f(viewPortSizeLocation, width, height));
    GLint currentTimeLocation;
    CHKERRGL(currentTimeLocation = glGetUniformLocation(shaderProgram, "currentTime"));
    CHKERRGL(glUniform1fv(currentTimeLocation, 1, &currentTime));
    CHKERRGL(glDrawArrays(GL_TRIANGLE_FAN, 0, 4));  // draw quad
}

int main(int argc, char *argv[])
{
    const char *fshader = "shaders/roots.frag";  // default
    if (argc > 1)
    {
        // Use first given argument as fragment shader file location
        fshader = argv[1];
    }
    
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }
    glfwSetErrorCallback(glfwError);
    // Minimum required version of OpenGL/GLSL is 4.2.
    // This is required for C-style array initializers used in GLSL shaders
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);  // 8 x multi-sampling
    
    GLFWwindow *window = glfwCreateWindow(800, 600, "Shadertoy", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    
    const GLenum glewError = glewInit();
    if (glewError != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(glewError));
        // TODO: Could use goto for cleanup code
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwSwapInterval(1);  // synchronize to display refresh rate
    
    // Setup basic OpenGL options
    CHKERRGL(glClearColor(0.0, 0.0, 0.0, 1.0));
    CHKERRGL(glEnable(GL_MULTISAMPLE));
    //CHKERRGL(glEnable(GL_DEPTH_TEST));
    //CHKERRGL(glEnable(GL_CULL_FACE));
    //CHKERRGL(glEnable(GL_BLEND));
    //CHKERRGL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    
    // Create shader program and attach it to current context
    GLuint shaderProgram = createGLShaderProgram(fshader);
    CHKERRGL(glUseProgram(shaderProgram));
    
    // Note: OpenGL versions 3.3+ require non-zero VAO to be bound.
    //       Otherwise, Invalid Operation is triggered by glDrawArrays(...) 
    GLuint vao;
    CHKERRGL(glGenVertexArrays(1, &vao));
    CHKERRGL(glBindVertexArray(vao));
    
    glfwSetTime(0.0);
    
    // Rendering loop
    while (!glfwWindowShouldClose(window))
    {
        GLint width, height;
        glfwGetFramebufferSize(window, &width, &height);
        
        CHKERRGL(glViewport(0, 0, width, height));
        CHKERRGL(glClear(GL_COLOR_BUFFER_BIT));
        //CHKERRGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        
        const GLdouble currentTime = glfwGetTime();
        
        // Render result of fragment shader to screen
        render(shaderProgram, width, height, currentTime);
        // Swap buffers from double buffering and display contents
        glfwSwapBuffers(window);
        glfwPollEvents();  // needed for window/input callbacks
    }
    
    // Cleanup
    CHKERRGL(glBindVertexArray(0));
    CHKERRGL(glDeleteVertexArrays(1, &vao));
    CHKERRGL(glUseProgram(0));
    CHKERRGL(glDeleteProgram(shaderProgram));
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return EXIT_SUCCESS;
}
