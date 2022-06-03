#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "shader.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Queue.h"
#include "Vertex.h"
#include "Color.h"

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

GLubyte image[IMAGE_WIDTH][IMAGE_HEIGHT][4];

// global variables used for rendering
// -----------------------------------
Shader *shader;
Shader *blurShader;
Shader *gaussianShader;

// Data structure to compare colors



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


// structure to hold config info
// -------------------------------
struct Config
{
    bool showWireframe = false;
    int brushSize = 30;
    float brushColor[3] = { 0.0f, 0.0f, 0.0f };
    int blurType = 0;
} config;


// function declarations
// ---------------------

void drawGui();

void setupPlane();

void FloodFill( int xPos, int yPos, Color targetColor, Color replacementColor );

void CalculateFrameRate( float lastFrame, float currentFrame );

// 2d texture
GLuint canvas;
GLuint squareVAO, VBO, EBO;
bool cursorIsHeldDown = false;
bool cursorIsDisabled = false;
glm::vec2 mousePos;
glm::vec2 clampedMousePos;

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

    shader = new Shader( "shaders/painting.vert", "shaders/painting.frag" );
    blurShader = new Shader( "shaders/painting.vert", "shaders/Blur.frag" );
    gaussianShader = new Shader( "shaders/painting.vert", "shaders/Gaussian.frag" );

    // init plane
    setupPlane();


    // Setup buffers
    glGenVertexArrays( 1, &squareVAO );
    glBindVertexArray( squareVAO );

    glGenBuffers( 1, &VBO );
    glBindBuffer( GL_ARRAY_BUFFER, VBO );
    glBufferData( GL_ARRAY_BUFFER, numberOfVertices * sizeof( Vertex ), &vertices, GL_STATIC_DRAW );

    glGenBuffers( 1, &EBO );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, EBO );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, numberOfIndices * sizeof( GLuint ), &indices, GL_STATIC_DRAW );

    // Setup attributes
    GLint PosAttrib{ glGetAttribLocation( shader->ID, "position" ) };
    glVertexAttribPointer( PosAttrib, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ),
                           (GLvoid *) 0 );
    glEnableVertexAttribArray( PosAttrib );
    GLint UVAttrib{ glGetAttribLocation( shader->ID, "texCoords" ) };
    glVertexAttribPointer( UVAttrib, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ),
                           (GLvoid *) offsetof( Vertex, TexCoords ));
    glEnableVertexAttribArray( UVAttrib );
    glBindVertexArray( 0 );


    // NEW! Enable SRGB framebuffer


    // Enable Face Culling
    glEnable( GL_CULL_FACE );
    glCullFace( GL_FRONT );

    // Dear IMGUI init
    // ---------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL( window, true );
    ImGui_ImplOpenGL3_Init( "#version 330 core" );

    // Create texture
    // -------------------------
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.


    // Initialize texture to be all white
    for ( int i = 0; i < IMAGE_WIDTH; i++ )
    {
        for ( int j = 0; j < IMAGE_HEIGHT; j++ )
        {

            image[i][j][0] = 255;
            image[i][j][1] = 255;
            image[i][j][2] = 255;
            image[i][j][3] = 255;
        }

    }
    // Setup texture
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    glGenTextures( 1, &canvas );
    glBindTexture( GL_TEXTURE_2D, canvas );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, IMAGE_WIDTH, IMAGE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, image );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    glBindTexture( GL_TEXTURE_2D, 0 );


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

        if ( config.blurType == 0 )
            shader->use();
        else if ( config.blurType == 1 )
            gaussianShader->use();
        else if ( config.blurType == 2 )
            blurShader->use();

        glBindVertexArray( squareVAO );
        glBindTexture( GL_TEXTURE_2D, canvas );
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

// queue data structure of vec2 for Flood Fill


Queue *Q = new Queue( IMAGE_WIDTH * IMAGE_HEIGHT );

Color GetImageColorFromUBytes( GLubyte r, GLubyte g, GLubyte b )
{
    return Color( r, g, b );
}


void FloodFill( int xPos, int yPos, Color targetColor, Color replacementColor )
{
    // Only fill if inside the bounds
    if ( xPos < 0 || xPos >= IMAGE_WIDTH || yPos < 0 || yPos >= IMAGE_HEIGHT )
        return;

    // Remove old queue
    delete Q;

    Q = new Queue( IMAGE_WIDTH * IMAGE_HEIGHT * 8 );
    // queue current pixel and start filling
    Q->enqueue( glm::vec2( yPos, xPos ));
    while ( !Q->isEmpty())
    {
        // Remove first pixel and check if inside bound
        glm::vec2 N = Q->dequeue();

        if ( N.x < 0 || N.x >= IMAGE_WIDTH || N.y < 0 || N.y >= IMAGE_HEIGHT )
            continue;

        // Compare colors of current pixel and target color. If they are the same, fill the pixel with replacement color
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


            auto right = image[(int) N.x + 1][(int) N.y];
            auto left = image[(int) N.x - 1][(int) N.y];
            auto up = image[(int) N.x][(int) N.y + 1];
            auto down = image[(int) N.x][(int) N.y - 1];
            // Get color values from neighboring pixels
            Color rightColor = GetImageColorFromUBytes( right[0], right[1], right[2] );
            Color leftColor = GetImageColorFromUBytes( left[0], left[1], left[2] );
            Color upColor = GetImageColorFromUBytes( up[0], up[1], up[2] );
            Color downColor = GetImageColorFromUBytes( down[0], down[1], down[2] );

            // enqueue neighbours that have the same color as target color
            if ( rightColor == targetColor )
                Q->enqueue( glm::vec2( N.x + 1, N.y ));
            if ( leftColor == targetColor )
                Q->enqueue( glm::vec2( N.x - 1, N.y ));
            if ( upColor == targetColor )
                Q->enqueue( glm::vec2( N.x, N.y + 1 ));
            if ( downColor == targetColor )
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
        cursorIsDisabled = io.WantCaptureMouse;

        ImGui::Begin( "Settings", NULL, ImGuiWindowFlags_NoCollapse );
        std::string fps2 = std::to_string( fps ) + " FPS";
        ImGui::Text( fps2.c_str());
        ImGui::Separator();
        ImGui::Checkbox( "Wireframe", &config.showWireframe );

        ImGui::Separator();
        ImGui::BeginGroup();
        ImGui::Text( "Blur Type" );
        if ( ImGui::Selectable( "Normal",
                                config.blurType == 0 ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None ))
        {
            config.blurType = 0;
        }
        if ( ImGui::Selectable( "Gaussian Blur",
                                config.blurType == 1 ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None ))
        {
            config.blurType = 1;
        }
        if ( ImGui::Selectable( "Custom Blur",
                                config.blurType == 2 ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None ))
        {
            config.blurType = 2;
        }

        ImGui::Separator();
        ImGui::EndGroup();

        ImGui::BeginGroup();
        ImGui::SliderInt( "Brush size", &config.brushSize, 0, 100 );
        ImGui::ColorPicker3( "Brush color", config.brushColor );
        ImGui::EndGroup();

        ImGui::End();

    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData());

    glEnable( GL_FRAMEBUFFER_SRGB );
}


void processInput( GLFWwindow *window )
{
    if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
        glfwSetWindowShouldClose( window, true );
}

void cursor_input_callback( GLFWwindow *window, double posX, double posY )
{

    static bool firstMouse = true;
    if ( firstMouse )
    {
        lastX = (float) posX;
        lastY = (float) posY;
        firstMouse = false;
    }

    // Clamp the mouse positions to be inside the window
    float min = 0;
    float maxX = SCR_WIDTH;
    float maxY = SCR_HEIGHT;
    posX = glm::clamp((float) posX, min, maxX - 1 );
    posY = glm::clamp((float) posY, min, maxY - 1 );

    // Normalize values between 0 and image width/height
    float x = ( IMAGE_WIDTH * (((float) ( posX ) - min ) / ((float) maxX - min )));
    float y = ( IMAGE_HEIGHT * (( SCR_HEIGHT - (float) posY - min ) / ((float) maxY - min )));
    clampedMousePos = { x, y };
    if ( cursorIsHeldDown )
    {
        float twoFifths = ( 1.0f / 5.0f ) * config.brushSize;
        float threeFifths = ( 4.0f / 5.0f ) * config.brushSize;

        int mouseY = clampedMousePos.y;
        int mouseX = clampedMousePos.x;
        int startY = (int) glm::clamp(( mouseY - ( config.brushSize / 2 )), 0, (int) IMAGE_HEIGHT );
        int startX = (int) glm::clamp( mouseX + ( config.brushSize / 2 ), 0, (int) IMAGE_WIDTH );


        // Draw "circle" shape
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
                    continue;
                }
                int actualY = (int) glm::clamp( startY + y, 0, (int) IMAGE_HEIGHT );
                int actualx = (int) glm::clamp( startX - x, 0, (int) IMAGE_WIDTH );

                image[(int) actualY][(int) actualx][0] = config.brushColor[0] * 255;
                image[(int) actualY][(int) actualx][1] = config.brushColor[1] * 255;
                image[(int) actualY][(int) actualx][2] = config.brushColor[2] * 255;
                image[(int) actualY][(int) actualx][3] = 255;
            }
        }

        // Update buffer with new image data
        glBindTexture( GL_TEXTURE_2D, canvas );
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE,
                         image );
        glBindTexture( GL_TEXTURE_2D, 0 );

    }

    lastX = (float) posX;
    lastY = (float) posY;

}


void key_input_callback( GLFWwindow *window, int button, int other, int action, int mods )
{
    // controls pause mode
    if ( glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS )
    {
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
    // Flood fill
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

        // Don't flood fill if the target color is the same as the replacement color
        if ( targetColor == replacementColor )
            return;

        FloodFill( clampedMousePos.x, clampedMousePos.y, targetColor, replacementColor );

        // Update the texture
        glBindTexture( GL_TEXTURE_2D, canvas );
        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE,
                         image );
        glBindTexture( GL_TEXTURE_2D, 0 );
    }
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback( GLFWwindow *window, double xoffset, double yoffset )
{
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback( GLFWwindow *window, int width, int height )
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.

    glViewport( 0, 0, width, height );
}

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

