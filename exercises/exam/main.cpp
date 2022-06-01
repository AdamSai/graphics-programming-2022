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

GLuint carLightTexture;
GLuint carWindowsTexture;
GLuint carWheelTexture;
Camera camera( glm::vec3( 0.0f, 1.6f, 5.0f ));

// plane setting
// --------------

constexpr GLuint dimensionsX = 50;
constexpr GLuint dimensionsY = 50;
int numberOfVertices = 0;
int numberOfIndices = 0;
Vertex vertices[dimensionsX * dimensionsY * 3];
unsigned int indices[( dimensionsX - 1 ) * ( dimensionsY - 1 ) * 6];


Shader *skyboxShader;
unsigned int skyboxVAO; // skybox handle
unsigned int cubemapTexture; // skybox texture handle

glm::mat4 lightSpaceMatrix;

// global variables used for control
// ---------------------------------
float lastX = (float) SCR_WIDTH / 2.0;
float lastY = (float) SCR_HEIGHT / 2.0;
float deltaTime;
bool isPaused = false; // stop camera movement when GUI is open

float lightRotationSpeed = 1.0f;

// structure to hold lighting info
// -------------------------------
struct Light
{
    Light( glm::vec3 position, glm::vec3 color, float intensity, float radius )
            : position( position ), color( color ), intensity( intensity ), radius( radius )
    {
    }

    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float radius;
};

// structure to hold config info
// -------------------------------
struct Config
{
    bool showWireframe = false;
    int brushSize = 30;
    bool blur = false;
} config;


// function declarations
// ---------------------
void setAmbientUniforms( glm::vec3 ambientLightColor );

void setLightUniforms( Light &light );

void setupForwardAdditionalPass();


void drawSkybox();


void drawObjects();

void drawGui();

unsigned int initSkyboxBuffers();

unsigned int loadCubemap( vector<std::string> faces );


// Water shader setting report
void setupPlane();

int GetVertexIndexAt( int xCoordinate, int yCoordinate );

void updateHeightmap();

GLubyte image[IMAGE_WIDTH][IMAGE_HEIGHT][4];

// 2d texture containing heightmap
GLuint heightmapTexture;
GLuint squareVAO, VBO, EBO;
bool holdingDownMouse = false;
glm::vec2 mousePos = glm::vec2( -1.0f, -1.0f );

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
    GLFWwindow *window = glfwCreateWindow( SCR_WIDTH, SCR_HEIGHT, "Water Water Water", NULL, NULL );
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
                           (GLvoid *) 0 ); // Note that we skip over the normal vectors
    glEnableVertexAttribArray( PosAttrib );
    GLint UVAttrib{ glGetAttribLocation( shader->ID, "texCoords" ) };
    glVertexAttribPointer( UVAttrib, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ),
                           (GLvoid *) offsetof( Vertex, TexCoords )); // Note that we skip over the normal vectors
    glEnableVertexAttribArray( UVAttrib );
//    GLint NormalAttrib{ glGetAttribLocation( shader->ID, "normal" ) };
//    glVertexAttribPointer( NormalAttrib, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ),
//                           (GLvoid *) offsetof( Vertex, Normal )); // Note that we skip over the normal vectors
//    glEnableVertexAttribArray( NormalAttrib );
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
    GLuint FramebufferName;
    glGenFramebuffers( 1, &FramebufferName );
    glBindFramebuffer( GL_FRAMEBUFFER, FramebufferName );



    // Create all black texture with size 500x500 and pass it to the shader
    for ( int i = 0; i < IMAGE_WIDTH; i++ )
    {
        for ( int j = 0; j < IMAGE_HEIGHT; j++ )
        {

            if ( i < 100 && j < 100 )
            {
                image[i][j][0] = 255;
                image[i][j][1] = 0;
                image[i][j][2] = 0;
                image[i][j][3] = 255;
            } else
            {
                image[i][j][0] = 255;
                image[i][j][1] = 255;
                image[i][j][2] = 255;
                image[i][j][3] = 1;
            }
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


    unsigned int rbo;
    glGenRenderbuffers( 1, &rbo );
    glBindRenderbuffer( GL_RENDERBUFFER, rbo );
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600 );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo );
    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
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
        if ( isPaused )
        {
            drawGui();
        } else
        {

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            {
                ImGui::Begin( "Settings" );
                std::string fps2 = to_string( fps ) + " FPS";
                ImGui::Text( fps2.c_str());
                ImGui::End();

            }
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData());
        }
        if ( config.showWireframe )
        {
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        } else
        {
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

        }
        glBindFramebuffer( GL_FRAMEBUFFER, FramebufferName );


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
        ImGui::Begin( "Settings" );
        ImGui::Checkbox( "Wireframe", &config.showWireframe );
        ImGui::Checkbox( "Blur", &config.blur );
        ImGui::SliderInt( "Brush size", &config.brushSize, 0, 100 );

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


// init the VAO of the skybox
// --------------------------
unsigned int initSkyboxBuffers()
{
    // triangles forming the six faces of a cube
    // note that the camera is placed inside of the cube, so the winding order
    // is selected to make the triangles visible from the inside
    float skyboxVertices[108]{
            // positions
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays( 1, &skyboxVAO );
    glGenBuffers( 1, &skyboxVBO );

    glBindVertexArray( skyboxVAO );
    glBindBuffer( GL_ARRAY_BUFFER, skyboxVBO );

    glBufferData( GL_ARRAY_BUFFER, sizeof( skyboxVertices ), &skyboxVertices, GL_STATIC_DRAW );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), (void *) 0 );

    return skyboxVAO;
}


void drawSkybox()
{
    // render skybox
//    glDepthFunc(
//            GL_LEQUAL );  // change depth function so depth test passes when values are equal to depth buffer's content
    skyboxShader->use();
    glm::mat4 projection = glm::perspective( glm::radians( camera.Zoom ), (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f,
                                             100.0f );
    glm::mat4 view = camera.GetViewMatrix();
    skyboxShader->setMat4( "projection", projection );
    skyboxShader->setMat4( "view", view );
    skyboxShader->setInt( "skybox", 0 );

    // skybox cube
    glBindVertexArray( skyboxVAO );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_CUBE_MAP, cubemapTexture );
    glDrawArrays( GL_TRIANGLES, 0, 36 );
    glBindVertexArray( 0 );
    glDepthFunc( GL_LESS ); // set depth function back to default
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

    if ( holdingDownMouse )
    {
        float min = 0;
        float maxX = SCR_WIDTH;
        float maxY = SCR_HEIGHT;
        posX = glm::clamp((float) posX, min, maxX );
        posY = glm::clamp((float) posY, min, maxY );
        // Normalize values between -1 and 0, so we can cover the screen
        float x = ( IMAGE_WIDTH * (((float) ( SCR_WIDTH - posX ) - min ) / ((float) maxX - min )));
        float y = ( IMAGE_HEIGHT * (((float) posY - min ) / ((float) maxY - min )));
        mousePos = { x, y };

        // TODO: Make this into a method

        float twoFifths = ( 1.0f / 5.0f ) * config.brushSize;
        float threeFifths = ( 4.0f / 5.0f ) * config.brushSize;

        int mouseY = mousePos.y;
        int mouseX = mousePos.x;
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
                    continue;
                }

                int actualY = (int) glm::clamp( startY + y, 0, (int) IMAGE_HEIGHT );
                int actualx = (int) glm::clamp( startX - x, 0, (int) IMAGE_WIDTH );

                int paintPos = actualY * ( SCR_WIDTH + 1 ) + actualx;
                image[(int) actualx][(int) actualY][0] = 0;
                image[(int) actualx][(int) actualY][1] = 0;
                image[(int) actualx][(int) actualY][2] = 0;
                image[(int) actualx][(int) actualY][3] = 255;
            }
        }

//        image[(int) mousePos.x][(int) mousePos.y][0] = 0;
//        image[(int) mousePos.x][(int) mousePos.y][1] = 0;
//        image[(int) mousePos.x][(int) mousePos.y][2] = 0;

        // mousePos /= 500;
        //mousePos = glm::clamp( mousePos, { 0.0f, 0.0f }, { 1.0f, 1.0f } );
// f

        // Update buffer with new image data
        glBindTexture( GL_TEXTURE_2D, heightmapTexture );
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, IMAGE_WIDTH, IMAGE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, image );
        glBindTexture( GL_TEXTURE_2D, 0 );

//        // resize depth attachment
//        gl.bindRenderbuffer( gl.RENDERBUFFER, this.renderBuffer );
//        gl.renderbufferStorage( gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, width, height );
//        gl.bindRenderbuffer( gl.RENDERBUFFER, null );
//
//        // update internal dimensions
//        this.rttwidth = width;
//        this.rttheight = height;


        std::cout << "posX " << mousePos.x << " posY " << mousePos.y << std::endl;
//        std::cout << "posX " << posX << " posY " << posY << std::endl;
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
    if ( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS )
    {
        holdingDownMouse = true;
        double xpos, ypos;
        //getting cursor position
        glfwGetCursorPos( window, &xpos, &ypos );

    } else if ( button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE )
    {

        holdingDownMouse = false;

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
    int halfX = dimensionsX / 2;
    int halfY = dimensionsY / 2;
    float maxX = 1 - halfX;
    float maxY = 1 - halfY;
    float minX = dimensionsX - 1 - halfX;
    float minY = dimensionsY - 1 - halfY;
    for ( int i = 0; i < dimensionsX; ++i )
    {
        for ( int j = 0; j < dimensionsY; ++j )
        {
            Vertex vertex;

            float tempX = ( j - halfX );
            float tempY = ( i - halfY );
            // Normalize values between -1 and 0 so we can cover the screen
            float x = ( 2 * (( tempX - minX ) / ( maxX - minX ))) - 1;
            float y = ( 2 * (( tempY - minY ) / ( maxY - minY ))) - 1;
            float z = 0;

//            float x = ( SCR_WIDTH * (((float) ( SCR_WIDTH - posX ) - min ) / ((float) maxX - min )));
//            float y = ( SCR_HEIGHT * (((float) posY - min ) / ((float) maxY - min )));
            vertex.Position = { x, y, z };
            vertex.TexCoords = { (float) i / ( dimensionsX - 1 ), (float) j / ( dimensionsY - 1 ) };
            vertex.Normal = { 0, 1, 0 };

            vertices[numberOfVertices++] = vertex;
//            std::cout << "UV: " << std::to_string( vertex.TexCoords.x ) << " " << std::to_string( vertex.TexCoords.y )
//                      << std::endl;
            std::cout << "XY: " << std::to_string( x ) << " " << std::to_string( y )
                      << std::endl;

        }
    }

    for ( int row = 0; row < dimensionsX - 1; ++row )
    {
        for ( int col = 0; col < dimensionsY - 1; ++col )
        {
            // d = row
            // w = col


            indices[numberOfIndices++] = dimensionsX * row + col;
            indices[numberOfIndices++] = dimensionsX * row + col + dimensionsY;
            indices[numberOfIndices++] = dimensionsX * row + col + dimensionsY + 1;

            indices[numberOfIndices++] = dimensionsX * row + col;
            indices[numberOfIndices++] = dimensionsX * row + col + dimensionsY + 1;
            indices[numberOfIndices++] = dimensionsX * row + col + 1;

        }
    }

    // Save neighbour info
    for ( int row = 0; row < dimensionsX - 1; ++row )
    {
        for ( int col = 0; col < dimensionsY - 1; ++col )
        {
            int left = GetVertexIndexAt( row - 1, col );
            int right = GetVertexIndexAt( row + 1, col );
            int up = GetVertexIndexAt( row, col + 1 );
            int down = GetVertexIndexAt( row, col - 1 );
//            std::cout << "Left: " << std::to_string( left ) << " Right: " << std::to_string( right ) << " Up: "
//                      << std::to_string( up ) <<
//                      " Down: " << std::to_string( down ) << std::endl;

        }
    }

}

int GetVertexIndexAt( int xCoordinate, int yCoordinate )
{
    return yCoordinate * ( dimensionsX + 1 ) + xCoordinate;
}

void updateHeightmap()
{

}
