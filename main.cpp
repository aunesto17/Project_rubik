/*
UCSP
COMPUTACION GRAFICA - 2024-II

ALEXANDER ARTURO BAYLON IBANEZ

Proyecto CUBO RUBIK
- definir cuantos vertices por cubo
- definir los tipos de cubos(3 colores, 2 colores, 1 color)
    - resto de colores gris o negro
    - solo se pintan las caras que corresponden
- despues tenemos que agregar texturas a cada cara
    - UCSP

11/11
1. modelado -> 26 cubos
2. rendering/apariencia -> color, textura
3. animacion 1 -> rotaciones camadas horizontal/vertical
4. animacion 2 -> conectar el cubo con un solver(c++)
    - solver:
        - analizar el estado actual
        - secuencia de rotaciones aplicadas a las camadas H/V
        - rotacion +- 90, 180, 270 grados
5. (PLUS) animacion propia
    - traer propuesta
    - presentacion de 6 slides
        - con ideas concretas
        - como se van a hacer los movimientos
        - formulas, fisica
        - todo es acumulativo
main.cpp - entry point for application

*/

// #define GLAD_GL_IMPLEMENTATION
// #include <glad/gl.h>
// #define GLFW_INCLUDE_NONE
// #include <GLFW/glfw3.h>

#include <iostream>
#include <math.h>
#include <random>
#include <vector> 

#include "vertex.h"
#include "matriz.h"
#include "rubik.h"
//#include "transform.h"
#include "helper.h"



void framebuffer_size_callback(GLFWwindow* window, int width, int height); //dimensionar la pantalla
void processInput(GLFWwindow *window); 

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

static void key_callback(GLFWwindow*, int, int, int, int); //callback para las teclas
unsigned int loadCubemap(vector<std::string> faces);

struct colorVec {
    float x, y, z;
    colorVec(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}
};

colorVec getRandomColor() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(0.0, 1.0);
    return colorVec(dis(gen), dis(gen), dis(gen));
}

// definimos las figuras
Camera camera;
CuboRubik cuboRubik(glfwGetTime(), camera);

float sbSize = camera.getMaxDistance() + 10.0f;

float lastFrame = 0.0f;
float deltaTime = 0.0f;
float currentFrame = 0.0f;

bool customAnimActive = 0;

//cuboRubik.setLastFrameTime();

// para el solver
std::vector<std::string> movreg;
std::vector<std::string> solvedCube;

// initial colors
colorVec backgroundColor(0.0f, 0.0f, 0.0f); // white background

// variable for current drawing mode
GLenum currentDrawMode = GL_TRIANGLES;

// para cada cuadrado de los cubos
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    //"layout (location = 2) in vec3 aTexCoord;\n"
    "layout (location = 2) in vec2 aTexCoord;\n"

    "out vec3 ourColor;\n"
    "out vec2 TexCoord;\n"
    "flat out int vFaceIndex;\n"

    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "uniform int faceIndex;\n"

    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
    //"   TexCoord = vec2(aTexCoord.x,aTexCoord.y);\n"    
    "   TexCoord = vec2(aTexCoord.x,aTexCoord.y);\n"
    "   vFaceIndex = faceIndex;\n"  
    "}\0";

const char *fragmentShaderTexSource = "#version 330 core\n"
    "out vec4 FragColor;\n"

    "in vec3 ourColor;\n"
    "in vec2 TexCoord;\n"
    //  textura letra U
    "uniform sampler2D texture1;\n"
    //  textura letra C
    "uniform sampler2D texture2;\n"
    //  textura letra S
    "uniform sampler2D texture3;\n"
    //  textura letra P
    "uniform sampler2D texture4;\n"
    //  textura logo
    "uniform sampler2D texture5;\n"

    "uniform int faceIndex;\n"

    "void main()\n"
    "{\n"
    //"   FragColor = texture(ourTexture, TexCoord);\n"
    "   vec4 texColor;\n"
    "   switch(faceIndex) {\n"
    "       case 0:  \n"
    "           texColor = texture(texture5, TexCoord);\n"
    "           break;\n"
    "       case 1:  \n"
    "           texColor = texture(texture4, TexCoord);\n"
    "           break;\n"
    "       case 2:  \n"
    "           texColor = texture(texture1, TexCoord);\n"
    "           break;\n"
    "       case 3:  \n"
    "           texColor = texture(texture2, TexCoord);\n"
    "           break;\n"
    "       case 4:  \n"
    "           texColor = texture(texture3, TexCoord);\n"
    "           break;\n"
    "       case 5:  \n"
    "           texColor = texture(texture5, TexCoord);\n"
    "           break;\n"
    "       default: \n"
    "           texColor = vec4(1.0);\n"
    "   }\n"
    
    //"   FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0f);\n"
    //"   FragColor = vec4(ourColor, 1.0f);\n"
    //"   FragColor = texColor * vec4(ourColor, 1.0f);\n"
        "// Handle transparency and color blending\n"
        "if(texColor.a < 0.1) {\n"
        "    // If mostly transparent, use the face color\n"
        "    FragColor = vec4(ourColor, 1.0);\n"
        "} else {\n"
        "    // Otherwise blend the texture with the face color\n"
        //"    vec3 blendedColor = mix(ourColor, texColor.rgb, texColor.a);\n"
        "    vec3 blendedColor = mix(ourColor, texColor.rgb, 0.25);\n"
        "    FragColor = vec4(blendedColor, 1.0);\n"
        "}\n"     
    "}\0";

const char *vertexShaderSBSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "out vec3 TexCoords;\n"
    "uniform mat4 projection;\n"
    "uniform mat4 view;\n"
    "void main()\n"
    "{\n"
    //"   TexCoords = aPos;\n"
    "   TexCoords = vec3(aPos.x, -aPos.y, aPos.z);\n"
    "   vec4 pos = projection * view * vec4(aPos, 1.0);\n"
    "   gl_Position = pos.xyww;\n"
    "}\0";

const char *fragmentShaderSBSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 TexCoords;\n"
    "uniform samplerCube skybox;\n"
    "void main()\n"
    "{\n"
    "   FragColor = texture(skybox, TexCoords);\n"
    "}\0";

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);  // Request 24-bit depth buffer

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	// plataforma que empaqueta a opengl
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Project_Rubik", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);  // hace que la ventana del programa aparezca delante de lo demas
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
	gladLoadGL(glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ---------------------------------------
    // vertices skybox
    // ---------------------------------------
    float skyboxVertices[] = {
        // positions          
        -sbSize,  sbSize, -sbSize,
        -sbSize, -sbSize, -sbSize,
         sbSize, -sbSize, -sbSize,
         sbSize, -sbSize, -sbSize,
         sbSize,  sbSize, -sbSize,
        -sbSize,  sbSize, -sbSize,

        -sbSize, -sbSize,  sbSize,
        -sbSize, -sbSize, -sbSize,
        -sbSize,  sbSize, -sbSize,
        -sbSize,  sbSize, -sbSize,
        -sbSize,  sbSize,  sbSize,
        -sbSize, -sbSize,  sbSize,

         sbSize, -sbSize, -sbSize,
         sbSize, -sbSize,  sbSize,
         sbSize,  sbSize,  sbSize,
         sbSize,  sbSize,  sbSize,
         sbSize,  sbSize, -sbSize,
         sbSize, -sbSize, -sbSize,

        -sbSize, -sbSize,  sbSize,
        -sbSize,  sbSize,  sbSize,
         sbSize,  sbSize,  sbSize,
         sbSize,  sbSize,  sbSize,
         sbSize, -sbSize,  sbSize,
        -sbSize, -sbSize,  sbSize,

        -sbSize,  sbSize, -sbSize,
         sbSize,  sbSize, -sbSize,
         sbSize,  sbSize,  sbSize,
         sbSize,  sbSize,  sbSize,
        -sbSize,  sbSize,  sbSize,
        -sbSize,  sbSize, -sbSize,

        -sbSize, -sbSize, -sbSize,
        -sbSize, -sbSize,  sbSize,
         sbSize, -sbSize, -sbSize,
         sbSize, -sbSize, -sbSize,
        -sbSize, -sbSize,  sbSize,
         sbSize, -sbSize,  sbSize
    };
    // ---------------------------------------
    // texturas
    // ---------------------------------------
    unsigned int textureU, textureC, textureS, textureP, textureUL;

    //Load each texture and check for errors
    textureU = loadTexture("letter-u.png");
    textureC = loadTexture("letter-c.png");
    textureS = loadTexture("letter-s.png");
    textureP = loadTexture("letter-p.png");
    textureUL = loadTexture("ucsp-logo.png");

    // Verify all textures loaded successfully
    if (textureU && textureC && textureS && textureP && textureUL) {
        std::cout << "All textures loaded successfully!" << std::endl;
    } else {
        std::cout << "Failed to load one or more textures!" << std::endl;
        // Handle error - maybe exit program or use default textures
    }

    // build and compile our shader program
    // ------------------------------------
    // VERTEX SHADER
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::0::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int vertexShaderSB = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShaderSB, 1, &vertexShaderSBSource, NULL);
    glCompileShader(vertexShaderSB);
    // check for shader compile errors
    glGetShaderiv(vertexShaderSB, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::SB::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
	
    // FRAGMENT SHADER   
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderTexSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    unsigned int fragmentShaderSB = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderSB, 1, &fragmentShaderSBSource, NULL);
    glCompileShader(fragmentShaderSB);
    // check for shader compile errors
    glGetShaderiv(fragmentShaderSB, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT_SB::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // LINK SHADERS and form a SHADER PROGRAM
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
	// check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::0::LINKING_FAILED\n" << infoLog << std::endl;
    }

    unsigned int shaderProgramSB = glCreateProgram();
    glAttachShader(shaderProgramSB, vertexShaderSB);
    glAttachShader(shaderProgramSB, fragmentShaderSB);
    glLinkProgram(shaderProgramSB);
	// check for linking errors
    glGetProgramiv(shaderProgramSB, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::SB::LINKING_FAILED\n" << infoLog << std::endl;
    }
    // delete used Shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShaderSB);
    glDeleteShader(fragmentShaderSB);

    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    std::vector<std::string> faces
    {
        // "stars/right.jpg",
        // "stars/left.jpg",
        // "stars/top.jpg",
        // "stars/bottom.jpg",
        // "stars/front.jpg",
        // "stars/back.jpg"
        "waters/right.jpg",
        "waters/left.jpg",
        "waters/top.jpg",
        "waters/bottom.jpg",
        "waters/front.jpg",
        "waters/back.jpg"
        // "stars2/right.jpg",
        // "stars2/left.jpg",
        // "stars2/top.jpg",
        // "stars2/bottom.jpg",
        // "stars2/front.jpg",
        // "stars2/back.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    //CuboRubik cuboRubik;
    cuboRubik.init();

    // tell opengl for each sampler to which texture it belongs
    glUseProgram(shaderProgram);
    int texture1Loc = glGetUniformLocation(shaderProgram, "texture1");
    int texture2Loc = glGetUniformLocation(shaderProgram, "texture2");
    int texture3Loc = glGetUniformLocation(shaderProgram, "texture3");
    int texture4Loc = glGetUniformLocation(shaderProgram, "texture4");
    int texture5Loc = glGetUniformLocation(shaderProgram, "texture5");
    int faceIndexLoc = glGetUniformLocation(shaderProgram, "faceIndex");

    // Set texture units
    glUniform1i(texture1Loc, 0); // Texture unit 0
    glUniform1i(texture2Loc, 1); // Texture unit 1
    glUniform1i(texture3Loc, 2); // Texture unit 2
    glUniform1i(texture4Loc, 3); // Texture unit 3
    glUniform1i(texture5Loc, 4); // Texture unit 4

    // Print uniform locations to verify they were found
    std::cout << "Texture uniform locations: " 
            << texture1Loc << ", " 
            << texture2Loc << ", "
            << texture3Loc << ", "
            << texture4Loc << ", "
            << texture5Loc << std::endl;
    

    // point and line sizes
	glPointSize(10.f);
    glLineWidth(5.f);

    // cam variables
    float cameraSpeed = 0.05f;
    bool unaPrueba=true;

    glUseProgram(shaderProgramSB);
    glUniform1i(glGetUniformLocation(shaderProgramSB, "skybox"), 0);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;   
        // input
        glfwSetKeyCallback(window, key_callback);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // render
        // ------

		// color del background
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);

        camera.updateCameraAnimation(deltaTime);

        if(!customAnimActive)
            cuboRubik.updateAnimation(currentFrame);
        else{
            customAnimActive = cuboRubik.updateCustomAnimation(currentFrame);
        }

        viewLoc = glGetUniformLocation(shaderProgram, "view");
        projLoc = glGetUniformLocation(shaderProgram, "projection");
        modelLoc = glGetUniformLocation(shaderProgram, "model");
        
        float viewMatrix[16], projMatrix[16];
        camera.getViewMatrix(viewMatrix);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspectRatio = (float)width / (float)height;
    
        camera.getPerspectiveMatrix(aspectRatio, projMatrix);
        
        glUseProgram(shaderProgram);

        // Send matrices to shader
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, projMatrix);
        
        // Model matrix (identity)
        float modelMatrix[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        glUniformMatrix4fv(modelLoc, 1, GL_TRUE, modelMatrix);

        
        // Bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureU);  // Top/Bottom texture

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureC);   // Left texture

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textureS);   // Front texture

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textureP);   // Right texture

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, textureUL);   // Back texture
        

        cuboRubik.draw(shaderProgram);

        // draw skybox as last
        viewLocSB = glGetUniformLocation(shaderProgramSB, "view");
        projLocSB = glGetUniformLocation(shaderProgramSB, "projection");

        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        glUseProgram(shaderProgramSB);
        camera.getViewMatrix(viewMatrix);
        //camera.getPerspectiveMatrix(aspectRatio, projMatrix);


        //viewMatrix = glm::mat4(glm::mat3(camera.getViewMatrix())); // remove translation from the view matrix
        //skyboxShader.setMat4("view", view);
        //skyboxShader.setMat4("projection", projection);
        glUniformMatrix4fv(viewLocSB, 1, GL_FALSE, viewMatrix);
        glUniformMatrix4fv(projLocSB, 1, GL_FALSE, projMatrix);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// mejor forma de detectar eventos de teclado y que la reaccion de pollEvents sea eficiente.
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //float currentFrame = glfwGetTime();
    //deltaTime = currentFrame - lastFrame;
    //lastFrame = currentFrame;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    // Camera controls
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        camera.moveForward(deltaTime);
        //std::cout<<"W delta: "<< deltaTime <<std::endl;
    }
        
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.moveBackward(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.moveLeft(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.moveRight(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.zoomIn(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.zoomOut(deltaTime);

    // scramble cube
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS){
        // empty current moves
        movreg.clear();
        movreg = cuboRubik.scrambleCube(30);
    }
        
    // solve cube
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS){
        string tempo1 = to_cube_not(movreg);
        movreg.clear();
        solvedCube=get_solution(tempo1);
        for(int i=0;i<solvedCube.size();++i){
            cout<<solvedCube[i]<<" ";
        }
        std::cout<<std::endl;
        //exeanimation(solvedCube,window);
        cuboRubik.moveFromList(solvedCube);
    }

    // animation sequence
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        customAnimActive = true;
        cuboRubik.setupExplosionAnimation();
        solvedCube.clear();
    }

    // reset cube
    if (key == GLFW_KEY_K && action == GLFW_PRESS) {
        cuboRubik.resetRubik();
        movreg.clear();
    }  

    // rotate cube faces
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
    {
        //cuboRubik.rotateFace('U', -90.0f);
        cuboRubik.rotateU();
        movreg.push_back("U");
    }
    if (key == GLFW_KEY_U && action == GLFW_PRESS)
    {
        //cuboRubik.rotateFace('U', -90.0f);
        cuboRubik.rotateUPrime();
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        cuboRubik.rotateL(); // clockwise
        movreg.push_back("L");
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        cuboRubik.rotateF(); // clockwise
        movreg.push_back("F");
    }
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
    {
        cuboRubik.rotateR(); // clockwise
        movreg.push_back("R");
    }
    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        cuboRubik.rotateB(); // clockwise
        movreg.push_back("B");
    }
    if (key == GLFW_KEY_H && action == GLFW_PRESS)
    {
        cuboRubik.rotateD(); // clockwise
        movreg.push_back("D");
    }
    // rotate cube slices
    if (key == GLFW_KEY_V && action == GLFW_PRESS)
    {
        cuboRubik.rotateSV(); // clockwise
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS)
    {
        cuboRubik.rotateSH(); // clockwise
    }
    if (key == GLFW_KEY_N && action == GLFW_PRESS)
    {
        cuboRubik.rotateSS(); // clockwise
    }



    
    // change display mode to lines only
    if(key == GLFW_KEY_I && action == GLFW_PRESS){
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    // change display mode to fill only
    if(key == GLFW_KEY_O && action == GLFW_PRESS){
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    // change display mode to points only 
    if(key == GLFW_KEY_P && action == GLFW_PRESS){
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    // change background color
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        backgroundColor = getRandomColor();}


}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}