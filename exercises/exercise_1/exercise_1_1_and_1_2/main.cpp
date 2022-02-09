#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

bool isWireframe = false;

void framebuffer_size_callback( GLFWwindow *window, int width, int height )
{
    glViewport( 0, 0, width, height );
}

void processInput( GLFWwindow *window )
{
    if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
        glfwSetWindowShouldClose( window, true );
}

void key_callback( GLFWwindow *window, int key, int scancode, int action, int mods )
{
    if ( key == GLFW_KEY_W && action == GLFW_PRESS )
    {
        if ( isWireframe )
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        else
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        isWireframe = !isWireframe;
    }
}

int main()
{
    glfwInit();
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    GLFWwindow *window = glfwCreateWindow( 800, 600, "LearnOpenGL", NULL, NULL );
    if ( window == NULL )
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent( window );
    glfwSetKeyCallback( window, key_callback );
    if ( !gladLoadGLLoader((GLADloadproc) glfwGetProcAddress ))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport( 0, 0, 800, 600 );
    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );


//    float vertices[] = {
//            -0.5f, -0.5f, 0.0f,
//            0.5f, -0.5f, 0.0f,
//            0.0f, 0.5f, 0.0f
//    };


    float vertices[] = {
            0.5f, 0.5f, 0.0f,  // top right
            0.5f, -0.5f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f,  // bottom left
            -0.5f, 0.5f, 0.0f   // top left
    };

    unsigned int indices[] = {  // note that we start from 0!
            0, 1, 3,   // first triangle
            1, 2, 3    // second triangle
    };

    // Define element object buffer
    unsigned int EBO;
    glGenBuffers( 1, &EBO );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );

    // Set up vertex shader
    unsigned int VBO;
    glGenBuffers( 1, &VBO );
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW );

    const char *vertexShaderSource = "#version 330 core\n"
                                     "layout (location = 0) in vec3 aPos;\n"
                                     "void main()\n"
                                     "{\n"
                                     "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                     "}\0";
    unsigned int vertexShader;
    vertexShader = glCreateShader( GL_VERTEX_SHADER );

    glShaderSource( vertexShader, 1, &vertexShaderSource, NULL );
    glCompileShader( vertexShader );

    // Check for errors
    int success;
    char infoLog[512];
    glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &success );

    if ( !success )
    {
        glGetShaderInfoLog( vertexShader, 512, NULL, infoLog );
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // set up fragment shader
    const char *fragmentShaderSource = "#version 330 core\n"
                                       "out vec4 FragColor;\n"
                                       "void main()\n"
                                       "{\n"
                                       " FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                                       " }\n";


    unsigned int fragmentShader;
    fragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( fragmentShader, 1, &fragmentShaderSource, NULL );
    glCompileShader( fragmentShader );

    // create shader program object and link vertex and fragment shader
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    glAttachShader( shaderProgram, vertexShader );
    glAttachShader( shaderProgram, fragmentShader );
    glLinkProgram( shaderProgram );

    // check for errors
    glGetProgramiv( shaderProgram, GL_LINK_STATUS, &success );
    if ( !success )
    {
        glGetProgramInfoLog( shaderProgram, 512, NULL, infoLog );
    }

    // specify how OpenGL should interpret the vertex data
    // 0 is for the attribute we want to configure. We set location to 0 in the shader
    // 3 is for the size of the vertex attribute which is vec3 (3 values)
    // GL_FLOAT is the type of data
    // GL_FALSE is if we should normalize
    // Next is the size between attributes. We have 3 floats so the size is the size of a float * 3
    // lastly is the offset of where the position data begins in the buffer


    unsigned int VAO;
    glGenVertexArrays( 1, &VAO );
    // ..:: Initialization code (done once (unless your object frequently changes)) :: ..
    // 1. bind Vertex Array Object
    glBindVertexArray( VAO );
    // 2. copy our vertices array in a buffer for OpenGL to use
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW );
    // 3. then set our vertex attributes pointers
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), (void *) 0 );
    glEnableVertexAttribArray( 0 );


    while ( !glfwWindowShouldClose( window ))
    {
        // input
        processInput( window );

        // render commands
        glClearColor( 0.5f, 0.0f, 0.5f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );


        // use our shader program when we want to render an object
        // draw object
        glUseProgram( shaderProgram );
        glBindVertexArray( VAO );
        glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 );

//        glDeleteShader( vertexShader );
//        glDeleteShader( fragmentShader );


        // check and call events and swap the buffers
        glfwSwapBuffers( window );
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

