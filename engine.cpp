#include "engine.h"

enum state {start, settings, play, over};
state screen;

// Colors
color originalFill, hoverFill, pressFill;


Engine::Engine() : keys() {
    this->initWindow();
    this->initShaders();
    this->initShapes();

    originalFill = {1, 0, 0, 1};
    hoverFill.vec = originalFill.vec + vec4{0.5, 0.5, 0.5, 0};
    pressFill.vec = originalFill.vec - vec4{0.5, 0.5, 0.5, 0};

    startTime = glfwGetTime();
}

Engine::~Engine() {}

unsigned int Engine::initWindow(bool debug) {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(width, height, "engine", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // OpenGL configuration
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    return 0;
}

void Engine::initShaders() {
    // load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Load shader into shader manager and retrieve it
    shapeShader = this->shaderManager->loadShader("../res/shaders/shape.vert", "../res/shaders/shape.frag",  nullptr, "shape");

    // Configure text shader and renderer
    textShader = shaderManager->loadShader("../res/shaders/text.vert", "../res/shaders/text.frag", nullptr, "text");
    fontRenderer = make_unique<FontRenderer>(shaderManager->getShader("text"), "../res/fonts/MxPlus_IBM_BIOS.ttf", 24);

    // Set uniforms
    textShader.setVector2f("vertex", vec4(100, 100, .5, .5));
    shapeShader.use();
    shapeShader.setMatrix4("projection", this->PROJECTION);

}

void Engine::initShapes() {
    //Initializing the sliders. Spawnbutton isnt a slider, but I found that I needed it for the program to run
    //there is a slider (shape instance) for the size of the target, the time you get per spawn round,
    //the number of targets, and the total time of the program.
    spawnButton = make_unique<Rect>(shapeShader, vec2{width/2,height/2}, vec2{100, 50}, color{1, 0, 0, 1});
    sizeButton = make_unique<Rect>(shapeShader, vec2{width/2,11.5*height/16}, vec2{80, 80}, color{1, 0, 0, 1});
    speedButton = make_unique<Rect>(shapeShader, vec2{width/2,8.5*height/16}, vec2{80, 80}, color{0, 1, 0, 1});
    countButton = make_unique<Rect>(shapeShader, vec2{width/2,5.5*height/16}, vec2{80, 80}, color{0, 0, 1, 1});
    timeButton = make_unique<Rect>(shapeShader, vec2{width/2,2.5*height/16}, vec2{80, 80}, color{1, 1, 1, 1});
}

void Engine::processInput() {
    glfwPollEvents();

    //variables for timing purposes
    float currentFrame = glfwGetTime();
    lastFrame = currentFrame;


    // Set keys to true if pressed, false if released
    for (int key = 0; key < 1024; ++key) {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            keys[key] = true;
        else if (glfwGetKey(window, key) == GLFW_RELEASE)
            keys[key] = false;
    }

    // Close window if escape key is pressed, extended to the stats window
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(stats, true);


    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);
    //if the user is in the start screen, an 's' input transitions to the settings screen,
    //and initializes the settings time variable
    if(screen == start) {
        if (keys[GLFW_KEY_S]) {
            screen = settings;
            settingsTime = glfwGetTime();
        }
    }

    //logic check that transitions the game if the user has spent 20 seconds on settings
    if(screen == settings) {
        float currentFrame = glfwGetTime();
        if (keys[GLFW_KEY_G] || currentFrame - settingsTime >= 20.0) {
            startTime = currentFrame;
            screen = play;
        }
    }

    //variable to adequately find the mouse's y component
    MouseY = height - MouseY;
    //variable for if the user presses the left mouse button
    bool press = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    //the logic for the size slider
    if(screen == settings) {
        //if the mouse is overlapping the size slider, it changes color
        if(sizeButton ->isOverlappingY(vec2(MouseX, MouseY))) {
            //variable to access the mouse's position
            vec2 mouse(MouseX, MouseY);
            sizeButton->setColor(hoverFill);

            //code to move slider with arrow keys
            if (keys[GLFW_KEY_LEFT])
                sizeButton->moveX(-3.80f);
            if (keys[GLFW_KEY_RIGHT])
                sizeButton->moveX(3.80f);

            //setting boundaries for the size slider
            if(sizeButton->getPosX() < 1.8*width/8){
                sizeButton->setPosX(1.8*width/8);
            }
            if(sizeButton->getPosX() > 6.2*width/8) {
                sizeButton->setPosX(6.2*width/8);
            }

            //logic to allow the mouse to also dictate the slider's position
            if(mouse.x > 1.8*width/8 && mouse.x < 6.2*width/8 && press) {
                sizeButton->setPosX(mouse.x);
            }

            //if the slider moves, update it's corresponding finalSize variable
            finalSize = value(sizeButton->getPosX(),"size");

        }
        else {
            //making sure that the color goes back to normal if not hovered on
            sizeButton->setColor(color{1, 0, 0, 1});
        }

        //slider for speed
        if(screen == settings) {
            if(speedButton ->isOverlappingY(vec2(MouseX, MouseY))) {
                //vector of mouse position
                vec2 mouse(MouseX, MouseY);
                speedButton->setColor(hoverFill);

                //allowing arrow keys to move the slider, if the mouse is hovering at the same horizontal
                //level
                if (keys[GLFW_KEY_LEFT])
                    speedButton->moveX(-3.80f);
                if (keys[GLFW_KEY_RIGHT])
                    speedButton->moveX(3.80f);

                //movement boundaries
                if(speedButton->getPosX() < 1.8*width/8){
                    speedButton->setPosX(1.8*width/8);
                }
                if(speedButton->getPosX() > 6.2*width/8) {
                    speedButton->setPosX(6.2*width/8);
                }
                //allowing the mouse to dictate slider position
                if(mouse.x > 1.8*width/8 && mouse.x < 6.2*width/8 && press) {
                    speedButton->setPosX(mouse.x);
                }
                //updating the finalSpeed variable
                finalSpeed = value(speedButton->getPosX(),"speed");
            }
            else {
                //if not hovered upon, change the color back to normal
                speedButton->setColor(color{0, 1, 0, 1});
            }
        }

        //slider for the targetCount Variable
        if(screen == settings) {
            if(countButton ->isOverlappingY(vec2(MouseX, MouseY))) {
                //if the user hovers over the horizontal of the slider, change the color
                vec2 mouse(MouseX, MouseY);
                countButton->setColor(hoverFill);

                //move the slider with the arrow keys
                if (keys[GLFW_KEY_LEFT])
                    countButton->moveX(-3.80f);
                if (keys[GLFW_KEY_RIGHT])
                    countButton->moveX(3.80f);

                //movement boundaries
                if(countButton->getPosX() < 1.8*width/8){
                    countButton->setPosX(1.8*width/8);
                }
                if(countButton->getPosX() > 6.2*width/8) {
                    countButton->setPosX(6.2*width/8);
                }

                //move the sliders with the mouse
                if(mouse.x > 1.8*width/8 && mouse.x < 6.2*width/8 && press) {
                    countButton->setPosX(mouse.x);
                }
                //updating the final count variable
                finalCount = value(countButton->getPosX(),"count");
            }
            else {
                //if not hovered upon, change the color back to normal
                countButton->setColor(color{0, 0, 1, 1});
            }
        }

        //slider for the time variable
        if(screen == settings) {
            if(timeButton ->isOverlappingY(vec2(MouseX, MouseY))) {
                //hovering over this button's y values causes a change in color
                vec2 mouse(MouseX, MouseY);
                timeButton->setColor(hoverFill);

                //moving the slider with the keys
                if (keys[GLFW_KEY_LEFT])
                    timeButton->moveX(-3.80f);
                if (keys[GLFW_KEY_RIGHT])
                    timeButton->moveX(3.80f);

                //movement boundaries
                if(timeButton->getPosX() < 1.8*width/8){
                    timeButton->setPosX(1.8*width/8);
                }
                if(timeButton->getPosX() > 6.2*width/8) {
                    timeButton->setPosX(6.2*width/8);
                }

                //mouse input changes the slider location
                if(mouse.x > 1.8*width/8 && mouse.x < 6.2*width/8 && press) {
                    timeButton->setPosX(mouse.x);
                }
                //updating the final time variable
                finalTime = value(timeButton->getPosX(),"time");
            }
            else {
                //making sure the color goes back to normal if not hovered on
                timeButton->setColor(color{1, 1, 1, 1});
            }
        }
    }

    //incrementing the misses variable if the user clicks not on a confetti circle
    if(screen == play) {
        for (unique_ptr<Shape>& circle : confetti) {
            if (!(circle->isOverlapping(vec2(MouseX, MouseY)))&&press) {
                misses++;
            }
        }
    }


    //iterating through the vector of shapes to see If the mouse is overlapping with any of them,
    //then outputting the time it took to get each;
    //calculating time
    currentFrame = glfwGetTime();
    lastFrame = currentFrame;
    for (unique_ptr<Shape>& circle : confetti) {
        //change the circle's color to let the user know that they are hovering in the right place
        if (circle->isOverlapping(vec2(MouseX, MouseY))) {
            circle->setColor(hoverFill);
        }
        //if the user clicks the circle, then move the circle off the map, to represent deleting it,
        //and incrementing the score variable
        if (circle->isOverlapping(vec2(MouseX, MouseY)) && press) {
            circle->setPosX(-1000);
            gotchaCount++;
        }
    }

    //mouse is clicked variable
    mousePressedLastFrame = press;

    //if the game is in the over state, and the user clicks the a key, then the game restarts, for a
    //continuous training loop
    if(screen == over) {
        if (keys[GLFW_KEY_A]) {
            screen = start;
        }
    }
}

void Engine::update() {
    //calculate time
    float currentFrame = glfwGetTime();
    lastFrame = currentFrame;


    //this logic represents the spawning of the next batch of targets. if the time from the last spawn
    //of confetti exceeds the user dictated finalSpeed variable, the next batch is spawned, and the last
    //spawn variable is updated
    if(currentFrame - lastSpawn >= finalSpeed) {
        confetti.clear();
        for(int i = 0; i < finalCount; i++) {
            spawnConfetti();
        }
        lastSpawn = currentFrame;
    }

    //variable for time spent in the game portion of the program, for help timing
    if (screen == play) {
        gameTime = currentFrame - startTime;
    }

    //this logic checks if while the program is in the game state and the time in the game portion
    //exceeds the user-defined finalTime variable, then the game is over, and the screen is switched to
    //the endgame screen
    if (screen == play && gameTime >= finalTime) {
        confetti.clear();
        screen = over;
    }
}

void Engine::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color
    glClear(GL_COLOR_BUFFER_BIT);

    // Set shader to draw shapes
    shapeShader.use();

    // Render differently depending on screen
    switch (screen) {
        case start: {
            //the main menu/entrance screen, welcoming the user. an s moves the screen to the next case
            string message = "Welcome to the Aim Trainer!";
            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height/2, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText("Press s to start!", width/2 - (12 * message.length()) + 80, (height/2)-40, projection, 1, vec3{1, 1, 1});
            break;
        }
        case settings: {
            //this screen case represents the user's choosing of the settings. the user is greeted, then
            //when they press g or twenty seconds have passed, then the screen is progressed to the game stage
            this->fontRenderer->renderText("Click G to start training!", 400, 750, projection, 1, vec3{1, 1, 1});
            this->fontRenderer->renderText("Adjust the sliders for difficulty", 375, 675, projection, 1, vec3{1, 1, 1});
            //sliders for the buttons

            //rendering the size slider
            shapeShader.use();
            sizeButton->setUniforms();
            sizeButton->draw();
            fontRenderer->renderText("Size", sizeButton->getPos().x - 40, sizeButton->getPos().y - 5, projection, 0.5, vec3{1, 1, 1});

            //rendering the shape slider
            shapeShader.use();
            speedButton->setUniforms();
            speedButton->draw();
            fontRenderer->renderText("speed", speedButton->getPos().x - 40, speedButton->getPos().y - 5, projection, 0.5, vec3{1, 1, 1});

            //rendering the target count slider
            shapeShader.use();
            countButton->setUniforms();
            countButton->draw();
            fontRenderer->renderText("targets", countButton->getPos().x - 40, countButton->getPos().y - 5, projection, 0.5, vec3{1, 1, 1});

            //rendering the time slider
            shapeShader.use();
            timeButton->setUniforms();
            timeButton->draw();
            fontRenderer->renderText("time", timeButton->getPos().x - 40, timeButton->getPos().y - 5, projection, 0.5, vec3{0, 0, 0});

            break;
        }
        case play: {
            //the case for when the game is being played
            //drawing all the confetti pieces initialized previously
            for (unique_ptr<Shape> &piece : confetti) {
                piece->setUniforms();
                piece->draw();
            }

            //a live countdown of the time remaining in the program, and your score
            fontRenderer->renderText(std::to_string(finalTime - gameTime), width/200, height/200, projection, 0.5, vec3{1, 1, 1});
            fontRenderer->renderText(std::to_string(gotchaCount), width/8, height/200, projection, 0.5, vec3{1, 1, 1});

            //calculating the user's clicking accuracy
            accuracy = gotchaCount / float(gotchaCount+misses);

            break;
        }
        case over: {
            //case for when the game is completed
            //a concluding message is rendered, then the user's stats.
            fontRenderer->renderText("Nice Training!", spawnButton->getPos().x - 30, spawnButton->getPos().y - 5, projection, 0.5, vec3{1, 1, 1});
            fontRenderer->renderText("Enter 'a' to Play Again!", spawnButton->getPos().x - 30, spawnButton->getPos().y - 30, projection, 0.5, vec3{1, 1, 1});
            fontRenderer->renderText("Time completed: " + std::to_string(finalTime), spawnButton->getPos().x - 30, spawnButton->getPos().y - 50, projection, 0.5, vec3{1, 1, 1});
            fontRenderer->renderText("Targets Clicked: " + std::to_string(gotchaCount), spawnButton->getPos().x - 30, spawnButton->getPos().y - 70, projection, 0.5, vec3{1, 1, 1});
            fontRenderer->renderText("Click Accuracy: " + std::to_string(accuracy) + "%", spawnButton->getPos().x - 30, spawnButton->getPos().y - 90, projection, 0.5, vec3{1, 1, 1});

            //[BUGGED] if stats window hasn't been initialized yet, then it is initialized and (hopefully)
            //the stats window is generated and shown - I commented this out to allow the main gameplay loop to
            //replay - everything gets a little messy when this is uncommented
             // if (stats == nullptr) {
             //     //calling the stats initialization funciton
             //     statsWindow();
             //     //I think this allows the computer to know what screen on which to render the text
             //     glfwMakeContextCurrent(stats);
             //     textShader.use();
             //     //showing the user's stats
             //     fontRenderer->renderText("Time completed: " + std::to_string(finalTime), newWidth/8, newHeight/8 , projection, 1.0, vec3(1, 1, 1));
             // }

            //resetting the score and misses for the next round
            gotchaCount = 0;
            misses = 0;
        }
    }

    glfwSwapBuffers(window);
}

void Engine::spawnConfetti() {
    //Reusing the spawnConfetti program to spawn a vector of shootable targets
    //random colors are selected
    vec2 pos = {rand() % (int)width, rand() % (int)height};
    //all targets are the same size, dictated by the user
    vec2 size = {finalSize, finalSize};

    color color = {float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), 1.0f};
    //populating the list of confetti
    confetti.push_back(make_unique<Circle>(shapeShader, pos, size, color));


}

bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}

//I tried to hijack this method for the stats window
bool Engine::statsShouldClose() {
    return glfwWindowShouldClose(stats);
}

GLenum Engine::glCheckError_(const char *file, int line) {
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        cout << error << " | " << file << " (" << line << ")" << endl;
    }
    return errorCode;
}

int Engine::value(float item, string type) {
    //a function to help find the value from the slider's position, by putting their positions into
    //a linear function with a predefined range
    //the ranges for the size, speed, target count, and time variables
    int lowSize = 5;
    int highSize = 105;

    int lowSpeed = 1;
    int highSpeed = 5;

    int lowCount = 1;
    int highCount = 6;

    int lowTime = 15;
    int highTime = 61;

    //this turns the slider's literal width (say, 375) into a float with a decimal, to represent
    //it's value out of 8, with the low bound of the slider being 1.8/8 and the high being 6.2/8
    item = item/175;


    if(type == "size") {
        //the function for finding the size value
        return 252/11 * (item - 1.8) + 5;
    }
    if(type == "speed") {
        //the function for finding the speed value
        return (0.92 * (item - 1.8) + 1);
    }
    if(type == "count") {
        //the function for finding the count value
        return 1.15 * (item - 1.8) + 1;
    }

    if(type == "time") {
        //the function for finding the time value
        return 10.5 * (item - 1.8) + 15;
    }
    else {
        return 0;
    }
}

    void Engine::statsWindow() {
    //function to initialize the stats window
    //initializing a new window
    stats = glfwCreateWindow(400, 400, "Game Over/Stats", nullptr, window);
    //error messsage
    if (!stats) {
        cout << "Could not load Final Screen";
    }

    glfwMakeContextCurrent(stats);
    glViewport(0, 0, newWidth, newHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}



