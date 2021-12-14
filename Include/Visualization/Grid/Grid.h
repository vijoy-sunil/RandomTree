#ifndef VISUALIZATION_GRID_H
#define VISUALIZATION_GRID_H

/* Be sure to include GLAD before GLFW. The include file 
 * for GLAD includes the required OpenGL headers behind 
 * the scenes (like GL/gl.h) so be sure to include GLAD 
 * before other header files that require OpenGL (like GLFW)

 * Because OpenGL is only really a standard/specification 
 * it is up to the driver manufacturer to implement the 
 * specification to a driver that the specific graphics 
 * card supports. Since there are many different versions 
 * of OpenGL drivers, the location of most of its functions 
 * is not known at compile-time and needs to be queried 
 * at run-time. It is then the task of the developer to 
 * retrieve the location of the functions he/she needs 
 * and store them in function pointers for later use.
 * 
 * GLAD is an open source library that manages all that 
 * cumbersome work we talked about.
*/
#include <glad/glad.h>
/* OpenGL is by itself a large state machine: 
 * a collection of variables that define how OpenGL should 
 * currently operate. The state of OpenGL is commonly 
 * referred to as the OpenGL CONTEXT. 
 * 
 * When using OpenGL, we often change its state by setting 
 * some options, manipulating some buffers and then render 
 * using the current context.
 * 
 * The first thing we need to do before we start creating 
 * stunning graphics is to create an OpenGL context and an 
 * application window to draw in. 
 * 
 * However, those operations are specific per operating 
 * system and OpenGL purposefully tries to abstract itself 
 * from these operations. This means we have to create a 
 * window, define a context, and handle user input all by 
 * ourselves.
 * 
 * GLFW gives us the bare necessities required for rendering 
 * goodies to the screen. It allows us to create an OpenGL 
 * context, define window parameters, and handle user input, 
 * which is plenty enough for our purposes.
*/
#include <GLFW/glfw3.h>
#include <vector>

/* enum to decide the type of data to be processed
*/
typedef enum{
    VERTEX, 
    COLOR
}dataType;
/* struct to represent color RGB
*/
typedef struct{
    float R;
    float G;
    float B;
}colorVal;

/* 2D grid class that abstracts all openGl funcitonalities 
 * required to set up and run render
*/
class GridClass{
    private:
        int scale;
        /* all cells within the grid will be made up of the same 
         * size; cellDim x cellDim
        */ 
        float cellDim;
        /* OpenGL only processes 3D coordinates when they're 
         * in a specific range between -1.0 and 1.0 on all 3 axes 
         * (x, y and z). All coordinates within this so called 
         * normalized device coordinates range will end up visible 
         * on your screen (and all coordinates outside this region 
         * won't).
        */        
        float axisMin, axisMax;
        /* As input to the graphics pipeline we pass in a list
         * of 3D coordinates that should form the desired shape
         * in an array here called VERTEX DATA; this vertex data 
         * is a collection of vertices. 
         * 
         * A vertex is a collection of data per 3D coordinate. 
         * This vertex's data is represented using VERTEX 
         * ATTRIBUTES that can contain any data we'd like.
        */
        std::vector<float> vertices;
        /* These indices tells the order to draw the quad using 
         * 4 vertices using EBO
        */
        std::vector<unsigned int> indices;
        /* RGBA format, define the color for all the grid cells.
         * We are not using vector here since we need to index
         * into a specific cell to set the colors. It is easier
         * to do this with arrays
        */
        float *color;
        /* Total #of elements in color array = 
         * 4 (RGBA) * 4(vertices per cell) * N * N
        */
        int colorArraySize;
        /* With the vertex data defined we'd like to send it as
         * input to the first process of the graphics pipeline: 
         * the vertex shader.
         * 
         * This is done by creating memory on the GPU where we 
         * store the vertex data, configure how OpenGL should 
         * interpret the memory and specify how to send the data 
         * to the graphics card. The vertex shader then processes 
         * as much vertices as we tell it to from its memory.
         * 
         * We manage this memory via so called vertex buffer 
         * objects (VBO) that can store a large number of vertices 
         * in the GPU's memory. The advantage of using those 
         * buffer objects is that we can send large batches of 
         * data all at once to the graphics card, and keep it 
         * there if there's enough memory left, without having 
         * to send data one vertex at a time. Sending data to 
         * the graphics card from the CPU is relatively slow, 
         * so wherever we can we try to send as much data as 
         * possible at once. Once the data is in the graphics 
         * card's memory the vertex shader has almost instant 
         * access to the vertices making it extremely fast
         *  
         * We will also be using EBO, An EBO is a buffer, just 
         * like a vertex buffer object, that stores indices 
         * that OpenGL uses to decide what vertices to draw. 
         * This also called indexed drawing. This can help us
         * reduce the number of vertices to define when drawing
         * quads using triangles
         * 
         * We will be using two VBOs, one for vertices and the
         * other for color
         */
        unsigned int VBOVertex, VBOColor, VAO, EBO;
        GLFWwindow* window;

        GLFWwindow* openGLBringUp(void);
        void genBufferObjects(void);
        void moveDataToGPU(dataType dtType);
        void setVertexAttribute(dataType dtType);
        void processInput(GLFWwindow* window);
        void openGLClose(void);

        void genCellVertices(float i, float j);
        void genCellVerticesWrapper(int i, int j);
        int getEboIdx(int i, int j);

    protected:
        /* the grid will be made up of NxN cells, the scale
         * factor scales up the window and everything in it
        */
        int N;
        /* cell position based on mouse click
        */
        int cellX, cellY;
        /* predefined color values
        */
        colorVal redVal, greenVal, blueVal, blackVal, whiteVal;

        void genCellColor(int i, int j, colorVal cVal, float alpha);
        void mouseAction(double mouseXPos, double mouseYPos);

        virtual void setObstacleCells(void) = 0;
        virtual void setStartAndEndCells(void) = 0;
        virtual void simulationStep(void) = 0;

    public:
        GridClass(int _N, int _scale, bool noStroke);
        ~GridClass(void);
        void runRender(void);
};
#endif /* VISUALIZATION_GRID_H
*/
