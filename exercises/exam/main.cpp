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

// screen settings
// ---------------
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;


// global variables used for rendering
// -----------------------------------
Shader *shader;

Model *carBodyModel;
Model *carPaintModel;
Model *carInteriorModel;
Model *carLightModel;
Model *carWindowsModel;
Model *carWheelModel;
Model *floorModel;
GLuint carBodyTexture;
GLuint carPaintTexture;
GLuint carLightTexture;
GLuint carWindowsTexture;
GLuint carWheelTexture;
Camera camera( glm::vec3( 0.0f, 1.6f, 5.0f ));

// plane setting
// --------------

constexpr GLuint dimensions = 50;
int numberOfVertices = 0;
int numberOfIndices = 0;
Vertex vertices[dimensions * dimensions * 3];
unsigned int indices[( dimensions - 1 ) * ( dimensions - 1 ) * 6];


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
    Config() : lights()
    {
        // Adding lights
        //lights.emplace_back(position, color, intensity, radius);

        // light 1
        lights.emplace_back( glm::vec3( -1.0f, 1.0f, -0.5f ), glm::vec3( 1.0f, 1.0f, 1.0f ), 1.0f, 0.0f );

        // light 2
        lights.emplace_back( glm::vec3( 1.0f, 1.5f, 0.0f ), glm::vec3( 0.7f, 0.2f, 1.0f ), 1.0f, 10.0f );
    }

    // ambient light
    glm::vec3 ambientLightColor = { 1.0f, 1.0f, 1.0f };
    float ambientLightIntensity = 0.25f;

    // material
    glm::vec3 reflectionColor = { 0.9f, 0.9f, 0.2f };
    float ambientReflectance = 0.75f;
    float diffuseReflectance = 0.75f;
    float specularReflectance = 0.5f;
    float specularExponent = 10.0f;
    float roughness = 0.5f;
    float metalness = 0.0f;

    std::vector<Light> lights;

    bool showWireframe = false;
    float waveStrength = 1.0f;
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

// 2d texture containing heightmap
GLuint heightmapTexture;
GLuint squareVAO, VBO, EBO;


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
    glfwSetScrollCallback( window, scroll_callback );

    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if ( !gladLoadGLLoader((GLADloadproc) glfwGetProcAddress ))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    shader = new Shader( "shaders/water.vert", "shaders/water.frag" );
    // init skybox
    vector<std::string> faces
            {
                    "skybox/right.tga",
                    "skybox/left.tga",
                    "skybox/top.tga",
                    "skybox/bottom.tga",
                    "skybox/front.tga",
                    "skybox/back.tga"
            };
    cubemapTexture = loadCubemap( faces );
    skyboxVAO = initSkyboxBuffers();
    skyboxShader = new Shader( "shaders/skybox.vert", "shaders/skybox.frag" );


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
                           (GLvoid *) 0 ); // Note that we skip over the normal vectors
    glEnableVertexAttribArray( UVAttrib );
    glBindVertexArray( 0 );

    // set up the z-buffer
    // -------------------
    glDepthRange( -1, 1 ); // make the NDC a right handed coordinate system, with the camera pointing towards -z
    glEnable( GL_DEPTH_TEST ); // turn on z-buffer depth test
    glDepthFunc( GL_LESS ); // draws fragments that are closer to the screen in NDC

    // NEW! Enable SRGB framebuffer
    glEnable( GL_FRAMEBUFFER_SRGB );


    // Enable Face Culling Report
    glEnable( GL_CULL_FACE );

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
    GLuint FramebufferName = 0;
    glGenFramebuffers( 1, &FramebufferName );
    glBindFramebuffer( GL_FRAMEBUFFER, FramebufferName );

    glGenTextures( 1, &heightmapTexture );
    glBindTexture( GL_TEXTURE_2D, heightmapTexture );

    // Create all black texture with size 500x500 and pass it to the shader
//    glTexImage2D( GL_TEXTURE_2D, 0, GL_R32F, 500, 500, 0, GL_RED, GL_FLOAT, 0 );
    std::vector<unsigned char> image( 1000 * 1000 * 3 /* bytes per pixel */ );
    for ( int i = 0; i < 1000 * 1000 * 3; i++ )
    {
        image[i] = 0;
    }
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 500, 500, 0, GL_RGB, GL_UNSIGNED_BYTE, &image[0] );

    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    // Set "heightmapTexture" as our colour attachement #0
    glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, heightmapTexture, 0 );

    // Set the list of draw buffers.
    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers( 1, DrawBuffers ); // "1" is the size of DrawBuffers

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

        glm::mat4 projection = glm::perspective( camera.Zoom, (float) SCR_WIDTH / (float) SCR_HEIGHT,
                                                 0.1f, 1000.0f );
        glm::mat4 model = glm::mat4( 1.0f );
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 viewProjection = projection * view;


        processInput( window );


        drawSkybox();
        glActiveTexture( GL_TEXTURE1 );
        glBindTexture( GL_TEXTURE_2D, heightmapTexture );

        shader->use();

        view = camera.GetViewMatrix();

        glBindVertexArray( squareVAO );
        // Pass the matrices to the shader
        shader->setMat4( "view", view );
        shader->setMat4( "projection", projection );
        shader->setMat4( "model", model );
        shader->setFloat( "time", currentFrame );
        shader->setFloat( "waveStrength", config.waveStrength );
//        GLuint texID = glGetUniformLocation( shader->ID, "heightmapTexture" );

        glDrawElements( GL_TRIANGLES, numberOfIndices, GL_UNSIGNED_INT, 0 );
        glBindVertexArray( 0 );


        if ( isPaused )
        {
            drawGui();
        }
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

    delete carBodyModel;
    delete carPaintModel;
    delete carInteriorModel;
    delete carLightModel;
    delete carWindowsModel;
    delete carWheelModel;
    delete floorModel;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void drawGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        ImGui::Begin( "Settings" );
        ImGui::Checkbox( "Wireframe", &config.showWireframe );
        ImGui::SliderFloat( "Wave Strength", &config.waveStrength, 0.0f, 10.0f );
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
    glDepthFunc( GL_EQUAL );


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

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap( vector<std::string> faces )
{
    unsigned int textureID;
    glGenTextures( 1, &textureID );
    glBindTexture( GL_TEXTURE_CUBE_MAP, textureID );

    int width, height, nrComponents;
    for ( unsigned int i = 0; i < faces.size(); i++ )
    {
        unsigned char *data = stbi_load( faces[i].c_str(), &width, &height, &nrComponents, 0 );
        if ( data )
        {
            glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                          data );
            stbi_image_free( data );
        } else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free( data );
        }
    }
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );

    glGenerateMipmap( GL_TEXTURE_CUBE_MAP );

    glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );

    return textureID;
}


void drawSkybox()
{
    // render skybox
    glDepthFunc(
            GL_LEQUAL );  // change depth function so depth test passes when values are equal to depth buffer's content
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

    lastX = (float) posX;
    lastY = (float) posY;

    if ( isPaused )
        return;

    // we use the handy camera class from LearnOpenGL to handle our camera
    camera.ProcessMouseMovement( xoffset, yoffset );
}


void key_input_callback( GLFWwindow *window, int button, int other, int action, int mods )
{
    // controls pause mode
    if ( glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS )
    {
        isPaused = !isPaused;
        glfwSetInputMode( window, GLFW_CURSOR, isPaused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED );
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
    int half = dimensions / 2;
    for ( int i = 0; i < dimensions; ++i )
    {
        for ( int j = 0; j < dimensions; ++j )
        {
            Vertex vertex;

            float x = j - half;
            float y = 0;
            float z = i - half;
            vertex.Position = { x, y, z };
            vertex.TexCoords = { (float) i / dimensions, (float) j / dimensions };

            vertices[numberOfVertices++] = vertex;
            std::cout << "UV: " << std::to_string( vertex.TexCoords.x ) << " " << std::to_string( vertex.TexCoords.y )
                      << std::endl;

        }
    }

    for ( int row = 0; row < dimensions - 1; ++row )
    {
        for ( int col = 0; col < dimensions - 1; ++col )
        {
            // d = row
            // w = col


            indices[numberOfIndices++] = dimensions * row + col;
            indices[numberOfIndices++] = dimensions * row + col + dimensions;
            indices[numberOfIndices++] = dimensions * row + col + dimensions + 1;

            indices[numberOfIndices++] = dimensions * row + col;
            indices[numberOfIndices++] = dimensions * row + col + dimensions + 1;
            indices[numberOfIndices++] = dimensions * row + col + 1;

        }
    }

    // Save neighbour info
    for ( int row = 0; row < dimensions - 1; ++row )
    {
        for ( int col = 0; col < dimensions - 1; ++col )
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
    return yCoordinate * ( dimensions + 1 ) + xCoordinate;
}

void updateHeightmap()
{

}
