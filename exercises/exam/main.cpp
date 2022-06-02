#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <vector>
#include <gl/GL.h>

// NEW! as our scene gets more complex, we start using more helper classes
//  I recommend that you read through the camera.h and model.h files to see if you can map the the previous
//  lessons to this implementation
#include "shader.h"
#include "camera.h"
#include "model.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "glm/gtc/type_ptr.hpp"

// glfw and input functions
// ------------------------
void processInput( GLFWwindow *window );

void scroll_callback( GLFWwindow *window, double xoffset, double yoffset );

void key_input_callback( GLFWwindow *window, int button, int other, int action, int mods );

void cursor_input_callback( GLFWwindow *window, double posX, double posY );

void framebuffer_size_callback( GLFWwindow *window, int width, int height );

// report
void mouse_button_callback( GLFWwindow *window, int button, int action, int mods );

// screen settings
// ---------------
const unsigned int SCR_WIDTH = 720;
const unsigned int SCR_HEIGHT = 720;

const unsigned int IMAGE_WIDTH = 1920;
const unsigned int IMAGE_HEIGHT = 1920;


// global variables used for rendering
// -----------------------------------
Shader *shader;

Camera camera( glm::vec3( 0.0f, 1.6f, 5.0f ));

struct Color
{
    GLubyte r;
    GLubyte g;
    GLubyte b;
    GLubyte a;

    Color( GLubyte r, GLubyte g, GLubyte b, GLubyte a )
    {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    Color( GLubyte r, GLubyte g, GLubyte b )
    {
        this->r = r;
        this->g = g;
        this->b = b;
    }

    // Override equality operator
    bool operator==( const Color &other ) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
};
// plane setting
// --------------

int numberOfVertices = 0;
int numberOfIndices = 0;
Vertex vertices[4];
unsigned int indices[6];


// global variables used for control
// ---------------------------------
float lastX = (float) SCR_WIDTH / 2.0;
float lastY = (float) SCR_HEIGHT / 2.0;
float deltaTime;
bool isPaused = false; // stop camera movement when GUI is open


// structure to hold config info
// -------------------------------
struct Config
{
    bool showWireframe = false;
    int brushSize = 30;
    bool blur = false;
    float brushColor[3] = { 0.0f, 0.0f, 0.0f };
} config;


// function declarations
// ---------------------

void setupForwardAdditionalPass();


void drawGui();

// Water shader setting report
void setupPlane();

void FloodFill( int xPos, int yPos, Color targetColor, Color replacementColor );

GLubyte image[IMAGE_WIDTH][IMAGE_HEIGHT][4];

// 2d texture containing heightmap
GLuint heightmapTexture;
GLuint squareVAO, VBO, EBO;
bool cursorIsHeldDown = false;
bool cursorIsDisabled = false;
glm::vec2 mousePos = glm::vec2( -1.0f, -1.0f );
glm::vec2 clampedMousePos = glm::vec2( 0.0f, 0.0f );

void CalculateFrameRate( float lastFrame, float currentFrame );

static float framesPerSecond = 0.0f;
static int fps;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow( SCR_WIDTH, SCR_HEIGHT, "Adam.paint", NULL, NULL );
    if ( window == NULL )
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent( window );
    glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );
    glfwSetCursorPosCallback( window, cursor_input_callback );
    glfwSetKeyCallback( window, key_input_callback );
    glfwSetMouseButtonCallback( window, mouse_button_callback );
    glfwSetScrollCallback( window, scroll_callback );

    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if ( !gladLoadGLLoader((GLADloadproc) glfwGetProcAddress ))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    shader = new Shader( "shaders/water.vert", "shaders/water.frag" );

    // init plane
    setupPlane();


    glGenVertexArrays( 1, &squareVAO );
    glBindVertexArray( squareVAO );

    glGenBuffers( 1, &VBO );
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, numberOfVertices * sizeof( Vertex ), &vertices, GL_STATIC_DRAW );

    glGenBuffers( 1, &EBO );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, numberOfIndices * sizeof( GLuint ), &indices, GL_STATIC_DRAW );
    GLint PosAttrib{ glGetAttribLocation( shader->ID, "position" ) };
    glVertexAttribPointer( PosAttrib, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ),
                           (GLvoid *) 0 );
    glEnableVertexAttribArray( PosAttrib );
    GLint UVAttrib{ glGetAttribLocation( shader->ID, "texCoords" ) };
    glVertexAttribPointer( UVAttrib, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ),
                           (GLvoid *) offsetof( Vertex, TexCoords ));
    glEnableVertexAttribArray( UVAttrib );
    glBindVertexArray( 0 );

    // set up the z-buffer
    // -------------------
//    glDepthRange( -1, 1 ); // make the NDC a right handed coordinate system, with the camera pointing towards -z
//    glEnable( GL_DEPTH_TEST ); // turn on z-buffer depth test
//    glDepthFunc( GL_LESS ); // draws fragments that are closer to the screen in NDC

    // NEW! Enable SRGB framebuffer
    glEnable( GL_FRAMEBUFFER_SRGB );


    // Enable Face Culling Report
//    glEnable( GL_CULL_FACE );

    // Dear IMGUI init
    // ---------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL( window, true );
    ImGui_ImplOpenGL3_Init( "#version 330 core" );

    // Create heightmap texture
    // -------------------------
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.


    // Create all black texture with size 500x500 and pass it to the shader
    for ( int i = 0; i < IMAGE_WIDTH; i++ )
    {
        for ( int j = 0; j < IMAGE_HEIGHT; j++ )
        {

            image[i][j][0] = 255;
            image[i][j][1] = 255;
            image[i][j][2] = 255;
            image[i][j][3] = 1;
        }

    }
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glGenTextures( 1, &heightmapTexture );
    glBindTexture( GL_TEXTURE_2D, heightmapTexture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, IMAGE_WIDTH, IMAGE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, image );
//    glTexImage2D( GL_TEXTURE_2D, 0, GL_R32F, 500, 500, 0, GL_RED, GL_FLOAT, 0 );
//    std::vector<unsigned char> pixels( 500 * 500 * 3 );
//    glGetTexImage( GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0] );


    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    glBindTexture( GL_TEXTURE_2D, 0 );


    // Always check that our framebuffer is ok
    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
        return false;

    // render loop
    // -----------
    while ( !glfwWindowShouldClose( window ))
    {
        glClearColor( 1.0f, 0.3f, 0.3f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        static float lastFrame = 0.0f;
        float currentFrame = (float) glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;


        processInput( window );


        glm::mat4 projection = glm::perspective( camera.Zoom, (float) SCR_WIDTH / (float) SCR_HEIGHT,
                                                 0.1f, 1000.0f );

        glm::mat4 model = glm::mat4( 1.0f );
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 viewProjection = projection * view;


        processInput( window );

        //    drawSkybox();


        shader->use();


        // Always check that our framebuffer is ok
        if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
            return false;

        glBindVertexArray( squareVAO );

        // Pass the matrices to the shader
        shader->setFloat( "time", currentFrame );
        shader->setBool( "blur", config.blur );
        shader->setMat4( "projection", projection );
        shader->setMat4( "view", view );
        shader->setMat4( "model", model );
        shader->setVec2( "mousePos", mousePos );
        GLuint texID = glGetUniformLocation( shader->ID, "heightmapTexture" );
        glUniform1i( texID, 0 );
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, heightmapTexture );
//        shader->setInt( "heightmapTexture", 1 );
//        glGetTexImage( GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, &pixels[0] );
//        glReadBuffer( GL_COLOR_ATTACHMENT0 );
//        std::cout << std::to_string( pixels[0] ) << std::endl;
        glDrawElements( GL_TRIANGLES, numberOfIndices, GL_UNSIGNED_INT, 0 );
        glBindVertexArray( 0 );
        glBindTexture( GL_TEXTURE_2D, 0 );


        CalculateFrameRate( deltaTime, lastFrame );

        drawGui();


        if ( config.showWireframe )
        {
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        } else
        {
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

        }


        // Render to the screen
        glBindFramebuffer( GL_FRAMEBUFFER, 0 );
        glfwSwapBuffers( window );
        glfwPollEvents();
    }

    // Cleanup
    // -------
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// queue data structure of vec2
class Queue
{
    std::vector<glm::vec2> data;
    int front, back;
    int size;
    int capacity;

public:
    Queue( int capacity )
    {
        this->capacity = capacity;
        data = std::vector<glm::vec2>( capacity );
        front = 0;
        back = 0;
        size = 0;
    }

    void enqueue( glm::vec2 item )
    {
        if ( size == capacity )
        {
            std::cout << "Queue is full" << std::endl;
            return;
        }
        data[back] = item;
        back = ( back + 1 ) % capacity;
        size++;
    }

    glm::vec2 dequeue()
    {
        if ( size == 0 )
        {
            std::cout << "Queue is empty" << std::endl;
            return glm::vec2( 0, 0 );
        }
        glm::vec2 item = data[front];
        front = ( front + 1 ) % capacity;
        size--;
        return item;
    }

    glm::vec2 peek()
    {
        if ( size == 0 )
        {
            std::cout << "Queue is empty" << std::endl;
            return glm::vec2( 0, 0 );
        }
        return data[front];
    }

    bool isEmpty()
    {
        return size == 0;
    }

    bool isFull()
    {
        return size == capacity;
    }

    // clear queue
    void clear()
    {
        front = 0;
        back = 0;
        size = 0;
    }
};

Queue *Q = new Queue( IMAGE_WIDTH * IMAGE_HEIGHT );

Color GetImageColorFromUBytes( GLubyte r, GLubyte g, GLubyte b )
{
    return Color( r, g, b );
}


void FloodFill( int xPos, int yPos, Color targetColor, Color replacementColor )
{
    if ( xPos < 0 || xPos >= IMAGE_WIDTH || yPos < 0 || yPos >= IMAGE_HEIGHT )
        return;
    delete Q;
    Q = new Queue( IMAGE_WIDTH * IMAGE_HEIGHT * 8 );
    Q->enqueue( glm::vec2( yPos, xPos ));
    while ( !Q->isEmpty())
    {
        glm::vec2 N = Q->dequeue();

        if ( N.x < 0 || N.x >= IMAGE_WIDTH || N.y < 0 || N.y >= IMAGE_HEIGHT )
            continue;

        int r = image[(int) N.x][(int) N.y][0];
        int g = image[(int) N.x][(int) N.y][1];
        int b = image[(int) N.x][(int) N.y][2];

        Color imgColor = Color( r, g, b );
        if ( imgColor == targetColor )
        {
            image[(int) N.x][(int) N.y][0] = replacementColor.r;
            image[(int) N.x][(int) N.y][1] = replacementColor.g;
            image[(int) N.x][(int) N.y][2] = replacementColor.b;
            image[(int) N.x][(int) N.y][3] = 255;


            Color right = GetImageColorFromUBytes( image[(int) N.x + 1][(int) N.y][0],
                                                   image[(int) N.x + 1][(int) N.y][1],
                                                   image[(int) N.x + 1][(int) N.y][2] );
            Color left = GetImageColorFromUBytes( image[(int) N.x - 1][(int) N.y][0],
                                                  image[(int) N.x - 1][(int) N.y][1],
                                                  image[(int) N.x - 1][(int) N.y][2] );
            Color up = GetImageColorFromUBytes( image[(int) N.x][(int) N.y + 1][0],
                                                image[(int) N.x][(int) N.y + 1][1],
                                                image[(int) N.x][(int) N.y + 1][2] );
            Color down = GetImageColorFromUBytes( image[(int) N.x][(int) N.y - 1][0],
                                                  image[(int) N.x][(int) N.y - 1][1],
                                                  image[(int) N.x][(int) N.y - 1][2] );

            // enqueue 4 neighbors
            if ( right == targetColor )
                Q->enqueue( glm::vec2( N.x + 1, N.y ));
            if ( left == targetColor )
                Q->enqueue( glm::vec2( N.x - 1, N.y ));
            if ( up == targetColor )
                Q->enqueue( glm::vec2( N.x, N.y + 1 ));
            if ( down == targetColor )
                Q->enqueue( glm::vec2( N.x, N.y - 1 ));


        }
    }
}


void CalculateFrameRate( float lastFrame, float currentFrame )
{
    // calculate frames per second
    static int frameCount = 0;
    static float lastTime = 0.0f;
    frameCount++;
    if ( currentFrame - lastTime >= 1.0f )
    {
        fps = frameCount;
        frameCount = 0;
        lastTime = currentFrame;
    }
}

void drawGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        //check if imgui wants to capture mouse
        ImGuiIO &io = ImGui::GetIO();
        if ( io.WantCaptureMouse )
        {
            cursorIsDisabled = true;
        } else
        {
            cursorIsDisabled = false;
        }
        ImGui::Begin( "Settings", NULL, ImGuiWindowFlags_NoCollapse );
        std::string fps2 = to_string( fps ) + " FPS";
        ImGui::Text( fps2.c_str());
        ImGui::Checkbox( "Wireframe", &config.showWireframe );
        ImGui::Checkbox( "Blur", &config.blur );
        ImGui::SliderInt( "Brush size", &config.brushSize, 0, 100 );
        ImGui::ColorPicker3( "Brush color", config.brushColor );
        ImGui::End();

    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData());

    glEnable( GL_FRAMEBUFFER_SRGB );
}

void setAmbientUniforms( glm::vec3 ambientLightColor )
{
    // ambient uniforms
    shader->setVec4( "ambientLightColor",
                     glm::vec4( ambientLightColor, glm::length( ambientLightColor ) > 0.0f ? 1.0f : 0.0f ));
}


void setupForwardAdditionalPass()
{
    // Remove ambient from additional passes
    setAmbientUniforms( glm::vec3( 0.0f ));

    // Enable additive blending
    glEnable( GL_BLEND );
    glBlendFunc( GL_ONE, GL_ONE );

    // Set depth test to GL_EQUAL (only the fragments that match the depth buffer are rendered)
//    glDepthFunc( GL_EQUAL );


}


void processInput( GLFWwindow *window )
{
    if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
        glfwSetWindowShouldClose( window, true );

    if ( isPaused )
        return;

    // movement commands
    if ( glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS )
        camera.ProcessKeyboard( FORWARD, deltaTime );
    if ( glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS )
        camera.ProcessKeyboard( BACKWARD, deltaTime );
    if ( glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS )
        camera.ProcessKeyboard( LEFT, deltaTime );
    if ( glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS )
        camera.ProcessKeyboard( RIGHT, deltaTime );
}

void cursor_input_callback( GLFWwindow *window, double posX, double posY )
{

    // camera rotation
    static bool firstMouse = true;
    if ( firstMouse )
    {
        lastX = (float) posX;
        lastY = (float) posY;
        firstMouse = false;
    }

    float xoffset = (float) posX - lastX;
    float yoffset = lastY - (float) posY; // reversed since y-coordinates go from bottom to top


    float min = 0;
    float maxX = SCR_WIDTH;
    float maxY = SCR_HEIGHT;
    posX = glm::clamp((float) posX, min, maxX );
    posY = glm::clamp((float) posY, min, maxY );
    // Normalize values between 0 and screen width/height
    float x = ( IMAGE_WIDTH * (((float) ( posX ) - min ) / ((float) maxX - min )));
    float y = ( IMAGE_HEIGHT * (( SCR_HEIGHT - (float) posY - min ) / ((float) maxY - min )));
    clampedMousePos = { x, y };
    if ( cursorIsHeldDown )
    {


        // TODO: Make this into a method

        float twoFifths = ( 1.0f / 5.0f ) * config.brushSize;
        float threeFifths = ( 4.0f / 5.0f ) * config.brushSize;

        int mouseY = clampedMousePos.y;
        int mouseX = clampedMousePos.x;
        int startY = (int) glm::clamp(( mouseY - ( config.brushSize / 2 )), 0, (int) IMAGE_HEIGHT );
        int startX = (int) glm::clamp( mouseX + ( config.brushSize / 2 ), 0, (int) IMAGE_WIDTH );


        for ( int y = 0; y <= config.brushSize; y++ )
        {
            for ( int x = 0; x <= config.brushSize; x++ )
            {

                // Don't draw corners
                if (
                        x < twoFifths && y < twoFifths ||
                        x >= threeFifths && y >= threeFifths ||
                        x < twoFifths && y >= threeFifths ||
                        x >= threeFifths && y < twoFifths
                        )
                {
                    // "3d" cube
//                    image[(int) actualY][(int) actualx][0] = config.brushColor[0] / 2 * 255;
//                    image[(int) actualY][(int) actualx][1] = config.brushColor[1] / 2 * 255;
//                    image[(int) actualY][(int) actualx][2] = config.brushColor[2] / 2 * 255;
//                    image[(int) actualY][(int) actualx][3] = 255;
                    continue;
                }
                int actualY = (int) glm::clamp( startY + y, 0, (int) IMAGE_HEIGHT );
                int actualx = (int) glm::clamp( startX - x, 0, (int) IMAGE_WIDTH );

                int paintPos = actualY * ( SCR_WIDTH + 1 ) + actualx;
                image[(int) actualY][(int) actualx][0] = config.brushColor[0] * 255;
                image[(int) actualY][(int) actualx][1] = config.brushColor[1] * 255;
                image[(int) actualY][(int) actualx][2] = config.brushColor[2] * 255;
                image[(int) actualY][(int) actualx][3] = 255;
            }
        }

//        image[(int) mousePos.x][(int) mousePos.y][0] = 0;
//        image[(int) mousePos.x][(int) mousePos.y][1] = 0;
//        image[(int) mousePos.x][(int) mousePos.y][2] = 0;


        // Update buffer with new image data
        glBindTexture( GL_TEXTURE_2D, heightmapTexture );
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE,
                         image );
        glBindTexture( GL_TEXTURE_2D, 0 );


        std::cout << "posX " << clampedMousePos.x << " posY " << clampedMousePos.y << std::endl;
    }

    lastX = (float) posX;
    lastY = (float) posY;

    if ( isPaused )
        return;

    // we use the handy camera class from LearnOpenGL to handle our camera
//    camera.ProcessMouseMovement( xoffset, yoffset );
}


void key_input_callback( GLFWwindow *window, int button, int other, int action, int mods )
{
    // controls pause mode
    if ( glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS )
    {
        isPaused = !isPaused;;
        glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
    }

}

void mouse_button_callback( GLFWwindow *window, int button, int action, int mods )
{
    if ( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !cursorIsDisabled )
    {
        cursorIsHeldDown = true;
        double xpos, ypos;
        //getting cursor position
        glfwGetCursorPos( window, &xpos, &ypos );

    } else if ( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE )
    {

        cursorIsHeldDown = false;

    }
    if ( button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS )
    {

        // create replacement color from brushColor
        Color replacementColor = { static_cast<GLubyte>(config.brushColor[0] * 255.f),
                                   static_cast<GLubyte>(config.brushColor[1] * 255.f),
                                   static_cast<GLubyte>(config.brushColor[2] * 255.f) };

        Color targetColor = { image[(int) clampedMousePos.y][(int) clampedMousePos.x][0],
                              image[(int) clampedMousePos.y][(int) clampedMousePos.x][1],
                              image[(int) clampedMousePos.y][(int) clampedMousePos.x][2] };

        std::cout << "Flood filling at: " << (int) clampedMousePos.x << ", " << (int) clampedMousePos.y
                  << " replacement color: " << std::to_string( replacementColor.r ) << ", "
                  << std::to_string( replacementColor.g ) << ", "
                  << std::to_string( replacementColor.b ) <<
                  " target color: " << std::to_string( targetColor.r ) << ", " << std::to_string( targetColor.g )
                  << ", " << std::to_string( targetColor.b ) << std::endl;

        if ( targetColor == replacementColor )
            return;
        FloodFill( clampedMousePos.x, clampedMousePos.y, targetColor, replacementColor );
        glBindTexture( GL_TEXTURE_2D, heightmapTexture );
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE,
                         image );
        glBindTexture( GL_TEXTURE_2D, 0 );
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback( GLFWwindow *window, double xoffset, double yoffset )
{
    camera.ProcessMouseScroll((float) yoffset );
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback( GLFWwindow *window, int width, int height )
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.

    glViewport( 0, 0, width, height );
}

// https://stackoverflow.com/questions/65199704/drawing-a-plane-with-vertex-buffer-object
void setupPlane()
{
    Vertex vertex1;
    vertex1.Position = { -1.0f, -1.0f, 0.0f };
    vertex1.TexCoords = { 0.0f, 0.0f };
    vertex1.Normal = { 0.0f, 0.0f, 1.0f };

    Vertex vertex2;
    vertex2.Position = { -1.0f, 1.0f, 0.0f };
    vertex2.TexCoords = { 0.0f, 1.0f };
    vertex2.Normal = { 0.0f, 0.0f, 1.0f };

    Vertex vertex3;
    vertex3.Position = { 1.0f, 1.0f, 0.0f };
    vertex3.TexCoords = { 1.0f, 1.0f };
    vertex3.Normal = { 0.0f, 0.0f, 1.0f };

    Vertex vertex4;
    vertex4.Position = { 1.0f, -1.0f, 0.0f };
    vertex4.TexCoords = { 1.0f, 0.0f };
    vertex4.Normal = { 0.0f, 0.0f, 1.0f };

    vertices[numberOfVertices++] = vertex1;
    vertices[numberOfVertices++] = vertex2;
    vertices[numberOfVertices++] = vertex3;
    vertices[numberOfVertices++] = vertex4;

    indices[numberOfIndices++] = 0;
    indices[numberOfIndices++] = 1;
    indices[numberOfIndices++] = 2;
    indices[numberOfIndices++] = 0;
    indices[numberOfIndices++] = 2;
    indices[numberOfIndices++] = 3;

}

