#ifndef GRAPHICS_ENGINE_H
#define GRAPHICS_ENGINE_H

#include <vector>
#include <memory>
#include <iostream>
#include <GLFW/glfw3.h>

#include "shader/shaderManager.h"
#include "font/fontRenderer.h"
#include "shapes/rect.h"
#include "shapes/shape.h"
#include "shapes/cube.h"
#include "shapes/circle.h"

using std::vector, std::unique_ptr, std::make_unique, glm::ortho, glm::mat4, glm::vec3, glm::vec4;

/**
 * @brief The Engine class.
 * @details The Engine class is responsible for initializing the GLFW window, loading shaders, and rendering the game state.
 */
class Engine {
private:
    /// @brief The actual GLFW window.
    GLFWwindow* window{};
    //new window pointer for stats after the end of the game. it starts as a nullptr,
    //so as to be called after the initial program starts
    GLFWwindow* stats = nullptr;

    /// @brief The width and height of the window.
    const unsigned int width = 1400, height = 800; // Window dimensions

    const unsigned int newWidth = 400, newHeight = 400; // Window dimensions for second window
    /// @brief The projection matrix
    const glm::mat4 projection = glm::ortho(0.0f, (float)width, 0.0f, (float)height);


    /// @brief Keyboard state (True if pressed, false if not pressed).
    /// @details Index this array with GLFW_KEY_{key} to get the state of a key.
    bool keys[1024];

    /// @brief Responsible for loading and storing all the shaders used in the project.
    /// @details Initialized in initShaders()
    unique_ptr<ShaderManager> shaderManager;

    /// @brief Responsible for rendering text on the screen.
    /// @details Initialized in initShaders()
    unique_ptr<FontRenderer> fontRenderer;

    //instances of buttons for the sliders. I needed spawnButton to declared for the base program to work
    unique_ptr<Shape> countButton;
    unique_ptr<Shape> timeButton;
    unique_ptr<Shape> speedButton;
    unique_ptr<Shape> spawnButton;
    unique_ptr<Shape> sizeButton;
    vector<unique_ptr<Shape>> confetti;

    //variables for the misses and accuracy of the user
    int misses;
    float accuracy;

    //variable for the number of targets that the user has hit
    int gotchaCount = 0;

    // Shaders
    Shader shapeShader;
    Shader textShader;

    //mouse variables
    double MouseX, MouseY;
    bool mousePressedLastFrame = false;

    //the variables for keeping track of time, so that the user has unlimited time in the main menu,
    //but limited time picking settings
    float startTime = 0.0;
    float lastSpawn = 0.0;
    float settingsTime = 0.0;
    float gameTime = 0.0;

    //I hard coded these values in so that the game would be able to run without a user's modifying the sliders
    //Though they change from user input (the sliders)
    float finalSize = value(width/2, "size");
    float finalSpeed = value(width/2, "speed");
    int finalCount = value(width/2, "count");
    int finalTime = value(width/2, "time");

    //Functions for helping the stats window close
    void statsWindow();
    bool statsShouldClose();

    /// @note Call glCheckError() after every OpenGL call to check for errors.
    GLenum glCheckError_(const char *file, int line);
    /// @brief Macro for glCheckError_ function. Used for debugging.
#define glCheckError() glCheckError_(__FILE__, __LINE__)

public:
    /// @brief Constructor for the Engine class.
    /// @details Initializes window and shaders.
    Engine();

    /// @brief Destructor for the Engine class.
    ~Engine();

    /// @brief Initializes the GLFW window.
    /// @return 0 if successful, -1 otherwise.
    unsigned int initWindow(bool debug = false);

    /// @brief Loads shaders from files and stores them in the shaderManager.
    /// @details Renderers are initialized here.
    void initShaders();

    /// @brief Initializes the shapes to be rendered.
    void initShapes();

    /// @brief Pushes back a new colored rectangle to the confetti vector.
    void spawnConfetti();

    /// @brief Processes input from the user.
    /// @details (e.g. keyboard input, mouse input, etc.)
    void processInput();

    /// @brief Updates the game state.
    /// @details (e.g. collision detection, delta time, etc.)
    void update();

    /// @brief Renders the game state.
    /// @details Displays/renders objects on the screen.
    void render();

    //for converting from a certain button's range to a value
    int value(float item, string type);


    /* deltaTime variables */
    float deltaTime = 0.0f; // Time between current frame and last frame
    float lastFrame = 0.0f; // Time of last frame (used to calculate deltaTime)

    // -----------------------------------
    // Getters
    // -----------------------------------

    /// @brief Returns true if the window should close.
    /// @details (Wrapper for glfwWindowShouldClose()).
    /// @return true if the window should close
    /// @return false if the window should not close
    bool shouldClose();

    /// Projection matrix used for 2D rendering (orthographic projection).
    /// We don't have to change this matrix since the screen size never changes.
    /// OpenGL uses the projection matrix to map the 3D scene to a 2D viewport.
    /// The projection matrix transforms coordinates in the camera space into normalized device coordinates (view space to clip space).
    /// @note The projection matrix is used in the vertex shader.
    // 4th quadrant
    mat4 PROJECTION = ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), -1.0f, 1.0f);
    // 1st quadrant
//        mat4 PROJECTION = ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));

};

#endif //GRAPHICS_ENGINE_H