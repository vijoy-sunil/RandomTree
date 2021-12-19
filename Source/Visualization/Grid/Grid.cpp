#include "../../../Include/Visualization/Grid/Grid.h"
#include "../../../Include/Visualization/Shader/Shader.h"
#include "../../../Include/Utils/Common.h"
#include <iostream>
#include <stdlib.h>
#include <cassert>

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        /* getting cursor position
        */
        glfwGetCursorPos(window, &xPos, &yPos);
        mouseClicked = true;
#if 0
    std::cout<<"xPos: "<<xPos<<" yPos: "<<yPos<<std::endl;
#endif
    }
}

/* this function will be called in the mouse action callback
 * function that is registered. the output of this funtion is
 * the cell position at which the mouse is clicked
 * if this is the screen space:
 * ------------------------------------- X axis
 * |(0,0)                           ((N)*scale-1,0)              
 * |                                    |
 * |                                    |
 * |                                    |
 * |(0, (N)*scale-1)               ((N)*scale-1, (N)*scale-1)
 * Y axis
 * 
 * What we need is to convert xPos and yPos to grid cell
 * coordinates, and set them to cellX, cellY.
 * 
 * NOTE: If the scale changes (by resizing the window while
 * running) then this functino fails to find the right cell
 * coords. Here, we have disabled window resizing.
*/
void GridClass::mouseAction(double mouseXPos, double mouseYPos){
    /* First we remove the scale factor from the position
     * returned. Next, xPos remains the same (based on the
     * above figure), but rate of change of yPos has to be
     * inverted
    */
    mouseXPos = mouseXPos/scale;
    mouseYPos = N - (mouseYPos/scale);
    /* compute grid cell position
    */
    cellX = (int)mouseXPos;
    cellY = (int)mouseYPos;
#if 0
    std::cout<<"cellX: "<<cellX<<" cellY: "<<cellY<<std::endl;
#endif
}

GridClass::GridClass(int _N, int _scale, bool noStroke){
    axisMin = -1.0;
    axisMax = 1.0;
    N = _N;
    assert(N % 2 == 0);

    scale = _scale;
    cellDim = (axisMax - axisMin)/N;
    
    /* init predefined color vals
    */
    redVal.R = 1.0;    redVal.G = 0.0;    redVal.B = 0.0;
    greenVal.R = 0.0;  greenVal.G = 1.0;  greenVal.B = 0.0;
    blueVal.R = 0.0,   blueVal.G = 0.0;   blueVal.B = 1.0;
    blackVal.R = 0.0;  blackVal.G = 0.0;  blackVal.B = 0.0;
    whiteVal.R = 1.0;  whiteVal.G = 1.0;  whiteVal.B = 1.0;

    colorArraySize = 16 * N * N;
    color = (float*)malloc(sizeof(float) * colorArraySize);

    /* opengl brinup routine
    */
    window = openGLBringUp();
    if(!window){
        std::cout<<"[ERROR] openGLBringUp() failed"<<std::endl;
        assert(false);
    }

    /* create all vertices for all grid cells
    */
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            genCellVerticesWrapper(i, j);
            genCellColor(i, j, whiteVal, 1.0);
        }
    }
    /* move data to gpu
    */
    moveDataToGPU(VERTEX);
    moveDataToGPU(COLOR);

    /* Right now we sent the input vertex data to the GPU 
     * and instructed the GPU how it should process the 
     * vertex data within a vertex and fragment shader. We're 
     * almost there, but not quite yet. OpenGL does not yet 
     * know how it should interpret the vertex data in memory 
     * and how it should connect the vertex data to the vertex 
     * shader's attributes.
     * 
     * The vertex shader allows us to specify any input we 
     * want in the form of vertex attributes and while 
     * this allows for great flexibility, it does mean we 
     * have to manually specify what part of our input data 
     * goes to which vertex attribute in the vertex shader. 
     * This means we have to specify how OpenGL should 
     * interpret the vertex data before rendering.
    */
    setVertexAttribute(VERTEX);
    setVertexAttribute(COLOR);
    /* We need this to enable alpha transperancy of our cells.
     * The glBlendFunc(GLenum sfactor, GLenum dfactor) function 
     * expects two parameters that set the option for the source 
     * and destination factor. 
    */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    /* enable/disable wireframe mode
    */
    if(noStroke)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

GridClass::~GridClass(void){
    free(color);
    openGLClose();
}

GLFWwindow* GridClass::openGLBringUp(void){
    /* Total screen space
    */
    const unsigned int screenWidth = N * scale;
    const unsigned int screenHeight = N * scale;
    /* main render window title
    */
    const char* windowTitle = "PATH FINDING";
    /* This fn initializes the GLFW library. This has to 
     * be done before most GLFW fns can be used
    */
    glfwInit();
    /* Configure GLFW, the first argument tells us what 
     * option we want to configure, where we can select the 
     * option from a large enum of possible options prefixed 
     * with GLFW_. 
     * 
     * The second argument is an integer that sets the value 
     * of our option.Here we'd like to tell GLFW that 3.3 is 
     * the OpenGL version we want to use. This way GLFW can 
     * make the proper arrangements when creating the OpenGL 
     * context. 
     * 
     * This ensures that when a user does not have the proper 
     * OpenGL version GLFW fails to run. We set the major and 
     * minor version both to 3. We also tell GLFW we want to 
     * explicitly use the core-profile. Telling GLFW we want 
     * to use the core-profile means we'll get access to a 
     * smaller subset of OpenGL features without backwards-
     * compatible features we no longer need.
    */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Note that on Mac OS X you need to add 
     * glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); 
     * to your initialization code for it to work.
    */
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    /* Disable window resize, we do this since it
     * changes the scale that we initially set
    */
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    /* Next we're required to create a window object. 
     * This window object holds all the windowing data and 
     * is required by most of GLFW's other functions.
     * 
     * This fn requires the window width and height as its 
     * first two arguments respectively. The third argument 
     * allows us to create a name for the window, we can 
     * ignore the remaining 2 arguments
     * 
     * The function returns a GLFWwindow object that we'll 
     * later need for other GLFW operations. 
    */
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, windowTitle, NULL, NULL);
    if (window == NULL){
        std::cout << "[ERROR] Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }
    /* Next we tell GLFW to make the context of our window 
     * the main context on the current thread.
    */
    glfwMakeContextCurrent(window);
    /* We have to tell GLFW we want to call this function 
     * on every mouse click
    */
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    /* GLAD manages function pointers for OpenGL so we want 
     * to initialize GLAD before we call any OpenGL function.
     * We pass GLAD the function to load the address of the 
     * OpenGL function pointers which is OS-specific. GLFW 
     * gives us glfwGetProcAddress that defines the correct 
     * function based on which OS we're compiling for.
    */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "[ERROR] Failed to initialize GLAD" << std::endl;
        return NULL;
    }

    /* gnereate buffers
    */
    genBufferObjects();
    return window;
}

void GridClass::genBufferObjects(void){
    /* A vertex array object (also known as VAO) can be 
     * bound just like a vertex buffer object, and any 
     * subsequent vertex attribute calls from that point 
     * on will be stored inside the VAO. This has the 
     * advantage that when configuring vertex attribute 
     * pointers you only have to make those calls once 
     * and whenever we want to draw the object, we can 
     * just bind the corresponding VAO. This makes switching 
     * between different vertex data and attribute 
     * configurations as easy as binding a different VAO. 
     * All the state we just set is stored inside the VAO.
     * 
     * NOTE: Core OpenGL requires that we use a VAO so it 
     * knows what to do with our vertex inputs. If we fail 
     * to bind a VAO, OpenGL will most likely refuse to 
     * draw anything.
     * 
     * To use a VAO all you have to do is bind the VAO 
     * using glBindVertexArray. From that point on we should 
     * bind/configure the corresponding VBO(s) and attribute 
     * pointer(s) and then unbind the VAO for later use. 
     * As soon as we want to draw an object, we simply bind 
     * the VAO with the preferred settings before drawing 
     * the object and that is it.
    */
    glGenVertexArrays(1, &VAO);
    /* A vertex buffer object is an OpenGL object. Just like 
     * any object in OpenGL, this buffer has a unique ID 
     * corresponding to thatbuffer, so we can generate one 
     * with a buffer ID using the glGenBuffers function. 
    */
    glGenBuffers(1, &VBOVertex);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &VBOColor);
}

/* bind the right buffer and move data to GPU, unbind after
 * data transfer
*/
void GridClass::moveDataToGPU(dataType dtType){
    if(dtType == VERTEX){
        /* Bind the Vertex Array Object first, then bind and 
         * set vertex buffer(s)
         * 
         * Bind VAO-----|
         *              |----Bind VBO
         *                      |
         *                      |-------- Transfer Vertex data
         *                      |
         *              |----Bind EBO
         *                      |
         *                      |-------- Transfer Index data
         *                      |
         * Unbind VAO-----------|
        */
        glBindVertexArray(VAO);
        /* OpenGL has many types of buffer objects and the 
         * buffer type of a vertex buffer object is 
         * GL_ARRAY_BUFFER. OpenGL allows us to bind to several 
         * buffers at once as long as they have a different 
         * buffer type. We can bind the newly created buffer to 
         * the GL_ARRAY_BUFFER target with the glBindBuffer 
         * function
         * 
         * From that point on any buffer calls we make (on the 
         * GL_ARRAY_BUFFER target) will be used to configure 
         * the currently bound buffer, which is VBO.
        */
        glBindBuffer(GL_ARRAY_BUFFER, VBOVertex);
        /* This fn copies the previously defined vertex data 
         * into the buffer's memory.
         * 
         * glBufferData is a function specifically targeted 
         * to copy user-defined data into the currently bound 
         * buffer. Its first argument is the type of the buffer 
         * we want to copy data into: the vertex buffer object 
         * currently bound to the GL_ARRAY_BUFFER target. The 
         * second argument specifies the size of the data (in 
         * bytes) we want to pass to the buffer; a simple sizeof 
         * of the vertex data suffices. The third parameter is 
         * the actual data we want to send.
         * 
         * The fourth parameter specifies how we want the graphics 
         * card to manage the given data. This can take 3 forms:
         * 
         * GL_STREAM_DRAW: the data is set only once and used 
         * by the GPU at most a few times.
         * GL_STATIC_DRAW: the data is set only once and used 
         * many times.
         * GL_DYNAMIC_DRAW: the data is changed a lot and used 
         * many times.
         * 
         * NOTE: If, for instance, one would have a buffer with 
         * data that is likely to change frequently, a usage type 
         * of GL_DYNAMIC_DRAW ensures the graphics card will 
         * place the data in memory that allows for faster writes.
         * 
         * As of now we stored the vertex data within memory on 
         * the graphics card as managed by a vertex buffer object 
         * named VBO. Next we want to create a vertex and fragment 
         * shader that actually processes this data.
        */
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), 
                    static_cast<void*>(vertices.data()), GL_STATIC_DRAW);
        /* Copy the index array into element buffer
        */
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), 
                    static_cast<void*>(indices.data()), GL_STATIC_DRAW);
        /* NOTE: do NOT unbind the BO while a VAO is active as the 
         * bound element buffer object IS stored in the VAO
        */
        /* NOTE: You can unbind the VAO afterwards so other VAO 
         * calls won't accidentally modify this VAO, but this 
         * rarely happens. Modifying other VAOs requires a call 
         * to glBindVertexArray anyways so we generally don't 
         * unbind VAOs (nor VBOs) when it's not directly necessary.
        */ 
        glBindVertexArray(0);
    }

    else if(dtType == COLOR){
        /* Bind the Vertex Array Object first, then bind and 
         * set vertex buffer(s)
         * 
         * Bind VAO-----|
         *              |----Bind VBOColor
         *                      |
         *                      |-------- Transfer Color data
         *                      |
         * Unbind VAO-----------|
        */
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBOColor);
        glBufferData(GL_ARRAY_BUFFER, colorArraySize * sizeof(float), color, GL_DYNAMIC_DRAW);
        glBindVertexArray(0);
    }
}

void GridClass::setVertexAttribute(dataType dtType){
    if(dtType == VERTEX){
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBOVertex);
        /* The first parameter specifies which vertex attribute 
         * we want to configure. For example, if we specified the 
         * location of the position vertex attribute in the 
         * vertex shader with layout (location = 0), it sets 
         * the location of the vertex attribute to 0 and if
         * we want to pass data to this vertex attribute, we 
         * pass in 0.
         * 
         * The next argument specifies the size of the vertex 
         * attribute. (example: if vec3, 3 is the size)
         * 
         * The third argument specifies the type of the data
         * 
         * The next argument specifies if we want the data to 
         * be normalized. If we're inputting integer data types 
         * (int, byte) and we've set this to GL_TRUE, the integer 
         * data is normalized to 0 (or -1 for signed data) and 
         * 1 when converted to float.
         * 
         * The fifth argument is known as the stride and tells 
         * us the space between consecutive vertex attributes. 
         * 
         * The last parameter is of type void* and thus requires 
         * that weird cast. This is the offset of where the 
         * position data begins in the buffer. For example,
         * if the position data is at the start of the data 
         * array this value is just 0.
         * 
         * NOTE: Each vertex attribute takes its data from memory 
         * managed by a VBO and which VBO it takes its data from 
         * (you can have multiple VBOs) is determined by the VBO 
         * currently bound to GL_ARRAY_BUFFER when calling 
         * glVertexAttribPointer
         */
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * (sizeof(float)), (void*)0);
        /* Now that we specified how OpenGL should interpret the 
        * vertex data we should also enable the vertex attribute 
        * with glEnableVertexAttribArray giving the vertex 
        * attribute location as its argument; vertex attributes 
        * are disabled by default.
        */
        glEnableVertexAttribArray(0);
        glBindVertexArray(0); 
    }
    else if(dtType == COLOR){
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBOColor);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * (sizeof(float)), (void*)0);
        glEnableVertexAttribArray(1);                  
        glBindVertexArray(0); 
    }
}

/* query GLFW whether relevant keys are pressed/released 
 * this frame and react accordingly. Here we check whether 
 * the user has pressed the escape key (if it's not pressed, 
 * glfwGetKey returns GLFW_RELEASE). If the user did press 
 * the escape key, we close GLFW by setting its WindowShouldClose 
 * property to true using glfwSetwindowShouldClose. The next 
 * condition check of the main while loop will then fail and 
 * the application closes.
*/
void GridClass::processInput(GLFWwindow* window){
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    /* step mode
    */
    if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        stepMode = true;

    /* Added input controls here
     * S - confirm start cell position (only once)
     * E - confirm end cell position
    */
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        /* NOTE: we cannot reset the start cell since the tree has
         * already grown from the root node, if we were to set another
         * node as a root node, then there exist a node who doesn't have
         * a parent - which conflicts the idea of a tree
        */
        startCellSet = true;
    if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        endCellSet = true;
}

void GridClass::openGLClose(void){
    /* de-allocate all resources once they've outlived 
     * their purpose
    */
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBOVertex);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBOColor);
    /* As soon as we exit the render loop we would like 
     * to properly clean/delete all of GLFW's resources 
     * that were allocated. We can do this via the glfwTerminate 
     * function that we call at the end of the main function.
    */
    glfwTerminate();
}

/* (i,j) will be the top left coordinates of a grid cell
 * pass in (i,j) to get vertices based on all variable 
 * parameters of the render screen. 
 * 
 * we need our grid cells to align as below
 * ----------------(+1)--------------
 * |        |       |       |       |
 * ----------------------------------
 * |        |       |       |       |
 * (-1)-------------0--------------(1)
 * |        |       |       |       |
 * ----------------------------------
 * |        |       |       |       |
 * ----------------(-1)--------------      
 * The function generates the 4 vertices required to draw
 * 2 triangles forming a quad. This also generates the 
 * indices vector that specifies the order in which these
 * vertices are to be drawn.
*/
void GridClass::genCellVertices(float i, float j){
    /* set top left first
    */
    vertices.push_back(i);
    vertices.push_back(j);
    vertices.push_back(0.0);
    /* bottom left
    */
    vertices.push_back(i);
    vertices.push_back(j - cellDim);
    vertices.push_back(0.0);
    /* bottom right
    */
    vertices.push_back(i + cellDim);
    vertices.push_back(j - cellDim);
    vertices.push_back(0.0);
    /* top right
    */
    vertices.push_back(i + cellDim);
    vertices.push_back(j);
    vertices.push_back(0.0);

    /* create indices to specify the order in which the above 
     * vertives are plotted
    */
    static int eboIdx = 0;
    indices.push_back(eboIdx);
    indices.push_back(eboIdx + 1);
    indices.push_back(eboIdx + 3);

    indices.push_back(eboIdx + 1);
    indices.push_back(eboIdx + 2);
    indices.push_back(eboIdx + 3);
    eboIdx += 4;
}

/* This function accepts (i,j) in grid cell coordinate format
 * We need to transform to screen space before invoking the 
 * sub function
*/
void GridClass::genCellVerticesWrapper(int i, int j){
    /* convert 2d coordinates to 1D, this will range from 
     * 0 to N*N-1
     * For example: let N = 4, cellDim = 0.5
     * So idx ranges from 0 to 15
    */
    int idx = i + N * j;
    /* first lets place idx 0 to N-1 cellDim apart
     * then (N) to 2(N)-1 cellsize apart, repeat this
     * till (N-1)N to ((N)(N))-1. Lets call
     * this row based placement
     *                          -1
     *                          |----------------------------------------
     *                          | 1.5,0   | 1.5,0.5 | 1.5,1.0 | 1.5,1.5 |
     *                          |----------------------------------------
     *                          | 1.0,0   | 1.0,0.5 | 1.0,1.0 | 1.0,1.5 |
     *                          |----------------------------------------
     *                          | 0.5,0   | 0.5,0.5 | 0.5,1.0 | 0.5,1.5 |
     *                          |----------------------------------------
     *                          | 0,0     | 0,0.5   | 0,1.0   | 0,1.5   | 
     * -1 ----------------------0---------------------------------------- +1
     *                          |
     *                          |
     *                          -1
     * Now that all points are in the screen space, they
     * are still in the first quadrant, we need to translate
     * them to lower left so that (0,0) is at the lower left
     * 
     * Finally, we need to shift by cellSize upwards to get
     * the top left vertex
    */
    float rowIdx = (idx / N) * cellDim;
    float colIdx = (idx % N) * cellDim;

    float x = rowIdx - (N/2) * cellDim;
    float y = colIdx - (N/2) * cellDim;
    
    y = y + cellDim;
    genCellVertices(x, y);
}

/* get ending eboIdx from cell location, makes it easier 
 * to set color of a cell based on its location
 * ---------------- .....
 * | 0,0  | 1,0   | .....
 * ---------------- .....
 *        ^       ^
 * eboIdx = 4     eboIdx = 8
*/
int GridClass::getEboIdx(int i, int j){
    return 4 + ((i + N * j) * 4);
}

/* generate color value for a cell by adding color to all 4
 * vertices. This fn will be called after generating all 4 
 * vertices of a cell
 *
 *         v0       v1       v2       v3
 *         x,y,z    x,y,z    x,y,z    x,y,z
 *         r,g,b,a  r,g,b,a  r,g,b,a  r,g,b,a
 *                                           ^
 *                                           |
 *                                           eboIdx = 4
*/
void GridClass::genCellColor(int i, int j, colorVal cVal, float alpha){
    int n = getEboIdx(i, j) * 4;
    int cellColorBatchSize = 16; /* 4 vertices, 4 color values
    */
    int k = n - cellColorBatchSize;
    /* set color for all 4 vertices  from top left,
     * bottom left, bottom right and top right
    */
    for(int i = 0; i < 4; i++){
        color[k++] = cVal.R;
        color[k++] = cVal.G;
        color[k++] = cVal.B;
        color[k++] = alpha;       
    }
}

/* this is the main render loop that runs the simulation at 
 * every time step
*/
void GridClass::runRender(void){
    /* |-----------------------------------------------------|
     * |                OVERRIDE IN CHILD CLASS              |
     * |-----------------------------------------------------|
    */
    /* initial cell states
    */
    setObstacleCells();
    moveDataToGPU(COLOR);
    /* |-----------------------------------------------------|
     * |                        END                          |
     * |-----------------------------------------------------|
    */
    /* the shader class we will be using here
    */
    ShaderClass Shader;
    /* We don't want the application to draw a single image 
     * and then immediately quit and close the window. We 
     * want the application to keep drawing images and handling 
     * user input until the program has been explicitly told 
     * to stop. 
     * 
     * For this reason we have to create a while loop, that we 
     * now call the RENDER LOOP, that keeps on running until we 
     * tell GLFW to stop.
     * 
     * The glfwWindowShouldClose function checks at the start 
     * of each loop iteration if GLFW has been instructed to 
     * close. If so, the function returns true and the render 
     * loop stops running, after which we can close the 
     * application.
    */
    while (!glfwWindowShouldClose(window)){
        /* We want to have some form of input control in GLFW 
         * and we can achieve this with several of GLFW's
         * input functions. We'll be using GLFW's glfwGetKey 
         * function that takes the window as input together 
         * with a key. The function returns whether this key 
         * is currently being pressed. We're creating a 
         * processInput function to keep all input code 
         * organized:
        */
        processInput(window);
        /* We want to clear the screen with a color of our 
         * choice. At the start of frame we want to clear the 
         * screen. Otherwise we would still see the results 
         * from the previous frame (this could be the effect 
         * you're looking for, but usually you don't). We can 
         * clear the screen's color buffer using glClear where 
         * we pass in buffer bits to specify which buffer we 
         * would like to clear. The possible bits we can set 
         * are GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT and 
         * GL_STENCIL_BUFFER_BIT. 
         * 
         * Note that we also specify the color to clear the 
         * screen with using glClearColor. Whenever we call 
         * glClear and clear the color buffer, the entire color 
         * buffer will be filled with the color as configured
         * by glClearColor
         * 
         * The glClearColor function is a state-setting function 
         * and glClear is a state-using function in that it uses 
         * the current state to retrieve the clearing color from.
        */
        glClearColor(whiteVal.R, whiteVal.G, whiteVal.B, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        /* Use the shader object that we linked using our shader 
         * files
        */
        Shader.use();
        /* |-----------------------------------------------------|
         * |                OVERRIDE IN CHILD CLASS              |
         * |-----------------------------------------------------|
        */
        /* set start and end goal cells
        */
        setStartAndEndCells();
        /* read cell states and set color array
        */
        simulationStep();
        /* |-----------------------------------------------------|
         * |                        END                          |
         * |-----------------------------------------------------|
        */        
        /* move color array to GPU
        */
        moveDataToGPU(COLOR);
        /* Do we want the data rendered as a collection of points, 
         * a collection of triangles or perhaps just one long line? 
         * Those hints are called primitives and are given to OpenGL 
         * while calling any of the drawing commands. Some of these 
         * hints are GL_POINTS, GL_TRIANGLES and GL_LINE_STRIP.
         * 
         * The glDrawArrays function takes as its first argument 
         * the OpenGL primitive type we would like to draw. 
         * The second argument specifies the starting index of the 
         * vertex array we'd like to draw; we just leave this at 0. 
         * The last argument specifies how many vertices we want to 
         * draw
         * 
         * Instead of glDrawArrays, we use glDrawElement because of
         * EBO. The first argument specifies the mode we want to draw 
         * in, similar to glDrawArrays. The second argument is the 
         * count or number of elements we'd like to draw. The third 
         * argument is the type of the indices which is of type 
         * GL_UNSIGNED_INT. The last argument allows us to specify 
         * an offset in the EBO (or pass in an index array, but 
         * that is when you're not using element buffer objects), 
         * but we're just going to leave this at 0.
        */
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        /* The glfwSwapBuffers will swap the color buffer (a large 
         * 2D buffer that contains color values for each pixel in 
         * GLFW's window) that is used to render to during this render 
         * iteration and show it as output to the screen.
         * 
         * When an application draws in a single buffer the resulting 
         * image may display flickering issues. This is because the 
         * resulting output image is not drawn in an instant, but drawn 
         * pixel by pixel and usually from left to right and top to 
         * bottom. Because this image is not displayed at an instant 
         * to the user while still being rendered to, the result may 
         * contain artifacts. To circumvent these issues, windowing 
         * applications apply a double buffer for rendering. The front 
         * buffer contains the final output image that is shown at the 
         * screen, while all the rendering commands draw to the back 
         * buffer. As soon as all the rendering commands are finished 
         * we swap the back buffer to the front buffer so the image can 
         * be displayed without still being rendered to, removing all 
         * the aforementioned artifacts.
        */
        glfwSwapBuffers(window);
        /* The glfwPollEvents function checks if any events are 
         * triggered (like keyboard input or mouse movement events), 
         * updates the window state, and calls the corresponding 
         * functions (which we can register via callback methods). 
        */
        glfwPollEvents();
    }
}
