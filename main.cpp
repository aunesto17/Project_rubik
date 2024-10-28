/*
UCSP
COMPUTACION GRAFICA - 2024-II

ALEXANDER ARTURO BAYLON IBANEZ

Proyecto CUBO RUBIK

*/


// #define GLAD_GL_IMPLEMENTATION
// #include <glad/gl.h>
// #define GLFW_INCLUDE_NONE
// #include <GLFW/glfw3.h>

#include <iostream>
#include <math.h>
#include <random>
#include <vector>
#include <filesystem>

#include "vertex.h"
#include "matriz.h"
#include "figura.h"
#include "transform.h"
#include "helper.h"
#include "camera.h"	


#include "stb_image.h"	// libreria para cargar imagenes

void framebuffer_size_callback(GLFWwindow* window, int width, int height); //dimensionar la pantalla
void processInput(GLFWwindow *window); 

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;



static void key_callback(GLFWwindow*, int, int, int, int);

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

void updateVertexBuffer();

// definimos las figuras
Cubo cubo;
Camera camera;

// Transform transC;
// Transform transE;
// Transform transP;

// initial colors
colorVec backgroundColor(1.0f, 1.0f, 1.0f); // white background
colorVec figureColor(1.0f, 0.0f, 0.0f); // red
colorVec triangleColor(0.0f, 1.0f, 0.0f); // green
colorVec pointColor(0.0f, 0.0f, 1.0f); // blue
colorVec starColor(1.0f, 1.0f, 0.0f); // yellow
colorVec pizzaColor(1.0f, 0.0f, 1.0f); // magenta

// variable for current drawing mode
GLenum currentDrawMode = GL_TRIANGLES;

// los Vertex Buffer Objects (VBO) almacenan multiples vertices que pueden ser enviados a la GPU de una sola vez
    // los Vertes Array Objects (VAO) almacenan multiples VBOs y pueden ser utilizados a demanda
unsigned int VBOs[5], VAOs[5], EBOs[5];

// current shaderprogram
unsigned int currentShaderProgram;

// para cada cuadrado de los cubos
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "layout (location = 2) in vec3 aTexCoord;\n"

    "out vec3 ourColor;\n"
    "out vec2 TexCoord;\n"

    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"

    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
    "   TexCoord = vec2(aTexCoord.x,aTexCoord.y);\n"    
    "}\0";

const char *fragmentShaderTexSource = "#version 330 core\n"
    "out vec4 FragColor;\n"

    "in vec3 ourColor;\n"
    "in vec2 TexCoord;\n"

    "uniform sampler2D ourTexture;\n"

    "void main()\n"
    "{\n"
    //"   FragColor = texture(ourTexture, TexCoord);\n"
    "   FragColor = vec4(ourColor, 1.0f);\n"     
    "}\0";



int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	// plataforma que empaqueta a opengl
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Project_06", NULL, NULL);
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
    // ---------------------------------------
	/*
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
	*/

    // texturas
    // ---------------------------------------
    unsigned int texture;
    // glGenTextures(cuantas texturas queremos generar, las guarda en este array(en este caso solo 1));
    glGenTextures(1, &texture);
    // bind the texture
    glBindTexture(GL_TEXTURE_2D, texture);
    // seteamos las opciones de envolvimiento/filtro de la textura
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
    // set the texture wrapping parameters
    //  1. texture target al objeto bindeado en el paso anterior
    //  2. nivel de mipmap (0 por defecto)
    //  3. formato para guardar la textura(la imagen es RGB entonces usamos GL_RGB)
    //  4. ancho de la textura (usamos los valores ya obtenidos)
    //  5. alto de la textura
    //  6. siempre 0
    //  7. formato de la imagen (RGB)
    //  8. tipo de dato de la imagen (unsigned byte)
    //  9. la imagen en si
    if(data){
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "fallo en cargar textura" << std::endl;
    }
    // liberar la memoria de la imagen
    stbi_image_free(data);


    glEnable(GL_DEPTH_TEST);


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
    // delete used Shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Get the location of the uniform variables for color
    int colorLoc = glGetUniformLocation(shaderProgram, "ourColor");
	
	
    // posiciones iniciales de las fig
    // for(int i=0; i<3; i++){
    //     transC.escala(vec3(0.75f, 0.75f, 0.75f), casa);
    // }
    // for(int i=0; i<6; i++){
    //     transC.traslacion(vec3(-0.1f, 0.0f, 0.0f), casa);
    // }
    // casa.verticesOrig = casa.vertices;


    //glGenVertexArrays(1, &VAO);
	glGenVertexArrays(5, VAOs);
    //glGenBuffers(1, &VBO);
	glGenBuffers(5, VBOs);
    glGenBuffers(5, EBOs);
	
	// puntos
	// glBindVertexArray(VAOs[0]);
    // //glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // // el buffer de openGL que acepta VBOs es el GL_ARRAY_BUFFER
	// glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    // // una vez asociado el VBO con el buffer entonces podemos ingresar la informacion de los vertices al buffer
    // glBufferData(GL_ARRAY_BUFFER, casa.vertices.size() * sizeof(float), &casa.vertices[0], GL_STATIC_DRAW);
    // // EBOs
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
    // // aqui utilizamos un ARRAY_BUFFER para los indices del cuadrado
    // // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesTri), indicesTri, GL_STATIC_DRAW);
    // // glVertexattribpointer es el encargado de decirle a openGL como debe interpretar los datos de los vertices:
    // // el primer argumento de los vertices es la posicion que se especifico en el shader(programa) con el layout(location = 0)
    // // el segundo argumento es el tamano del atributo, en este caso es un vec3, por lo que tiene 3 valores
    // // el tercer argumento es el tipo de dato que se esta ingresando, en este caso es un float
    // // el cuarto argumento es si se quiere normalizar los datos, en este caso no se normaliza
    // // el quinto argumento es el tamano de los datos, en este caso es 3 * sizeof(float)
    // // el sexto argumento es el offset de los datos, en este caso es 0
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);

    // cubos
    glBindVertexArray(VAOs[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, cubo.vertices.size() * sizeof(float), &cubo.vertices[0], GL_STATIC_DRAW);
    // EBOs
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubo.indices.size() * sizeof(float), std::data(cubo.indices), GL_STATIC_DRAW);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);


    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    //glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPointSize(10.f);
    glLineWidth(5.f);

    currentShaderProgram = 4;
    

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        //processInput(window);

        glfwSetKeyCallback(window, key_callback);

        // render
        // ------
		// color del background
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // naranja
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        //glUseProgram(shaderProgram);
        viewLoc = glGetUniformLocation(shaderProgram, "view");
        projLoc = glGetUniformLocation(shaderProgram, "projection");
        modelLoc = glGetUniformLocation(shaderProgram, "model");
        
        // Set up camera
        //setupCamera(float(SCR_WIDTH), float(SCR_HEIGHT));
        // Use the shader program
        
        //float distance = 10.0f;  // Adjust this based on your scene size
        float horizontalRadius = camera.distance * cos(toRadians(camera.elevation));
        float eye[3] = {
            // distance * 0.866f,  // X = distance * cos(45°) * sin(35.264°)
            // distance * 0.866f,  // Y = distance * sin(45°) * sin(35.264°)
            // distance * 0.866f   // Z = distance * cos(35.264°)
            horizontalRadius * sin(toRadians(camera.rotationAngle)),
            camera.distance * sin(toRadians(camera.elevation)),
            horizontalRadius * cos(toRadians(camera.rotationAngle))
        };
        // float center[3] = {0.0f, 0.0f, 0.0f};  // Looking at origin
        // float up[3] = {-0.866f, 0.866f, 0.0f}; // Adjusted up vector for isometric view
        // float viewMatrix[16];

        float center[3] = {0.0f, 0.0f, 0.0f};  // Looking at origin
        float up[3] = {0.0f, 1.0f, 0.0f};      // World up vector
        float viewMatrix[16];
        
        // Calculate view matrix
        lookAt(eye, center, up, viewMatrix);
        
        // Calculate orthographic projection matrix
        float aspect = width / height;
        float size = 2.0f;  // Adjust this to control zoom level
        float projMatrix[16];
        
        // Set up orthographic projection
        ortho(-size * aspect, size * aspect,    // left, right
              -size, size,                      // bottom, top
              0.1f, 100.0f,                     // near, far
              projMatrix);
        
        glUseProgram(shaderProgram);

        // Send matrices to shader
        glUniformMatrix4fv(viewLoc, 1, GL_TRUE, viewMatrix);
        glUniformMatrix4fv(projLoc, 1, GL_TRUE, projMatrix);
        
        // Model matrix (identity)
        float modelMatrix[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        glUniformMatrix4fv(modelLoc, 1, GL_TRUE, modelMatrix);
        

        //glUniform3f(colorLoc, pointColor.x, pointColor.y, pointColor.z);
        glBindVertexArray(VAOs[0]);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    //glDeleteVertexArrays(1, &VAO);
	glDeleteVertexArrays(4, VAOs);
    //glDeleteBuffers(1, &VBO);
	glDeleteBuffers(4, VBOs);
    // glDeleteProgram(shaderProgram);
	// glDeleteProgram(shaderProgram1);
	// glDeleteProgram(shaderProgram2);
    // glDeleteProgram(shaderProgram3);


    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) // corregir el error logico de esta funcion cuando se invoca en el render loop
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
	
	if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
		glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
	}
}

void updateVertexBuffer() {
    
}

// mejor forma de detectar eventos de teclado y que la reaccion de pollEvents sea eficiente.
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    // // rotaciones X, Y, Z
    // if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    // {
    //     std::cout << "Key: Z - Rotacion en X" << std::endl;
    //     transC.rotacionX(15, casa);
    //     transE.rotacionX(15, estrella);
    //     transP.rotacionX(15, pizza);
    //     updateVertexBuffer();
    // }
    // if (key == GLFW_KEY_X && action == GLFW_PRESS)
    // {
    //     std::cout << "Key: X - Rotacion en Y" << std::endl;
    //     transC.rotacionY(15, casa);
    //     transE.rotacionY(15, estrella);
    //     transP.rotacionY(15, pizza);
    //     updateVertexBuffer();
    // }
    // if (key == GLFW_KEY_C && action == GLFW_PRESS)
    // {
    //     std::cout << "Key: C - Rotacion en Z" << std::endl;
    //     transC.rotacionZ(15, casa);
    //     transE.rotacionZ(15, estrella);
    //     transP.rotacionZ(15, pizza);
    //     updateVertexBuffer();
    // }
    // // inversas de rotaciones
    // if (key == GLFW_KEY_V && action == GLFW_PRESS)
    // {
    //     std::cout << "Key: V - Rotacion en X inversa" << std::endl;
    //     transC.rotacionX_i(casa);
    //     transE.rotacionX_i(estrella);
    //     transP.rotacionX_i(pizza);
    //     updateVertexBuffer();
    // }

    // if (key == GLFW_KEY_B && action == GLFW_PRESS)
    // {
    //     std::cout << "Key: B - Rotacion en Y inversa" << std::endl;
    //     transC.rotacionY_i(casa);
    //     transE.rotacionY_i(estrella);
    //     transP.rotacionY_i(pizza);
    //     updateVertexBuffer();
    // }
    // if (key == GLFW_KEY_N && action == GLFW_PRESS)
    // {
    //     std::cout << "Key: N - Rotacion en Z inversa" << std::endl;
    //     transC.rotacionZ_i(casa);
    //     transE.rotacionZ_i(estrella);
    //     transP.rotacionZ_i(pizza);
    //     updateVertexBuffer();
    // }

    // traslacion
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        camera.elevation = 35.264f;
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        camera.elevation = -35.264f;
    }

    // if (key == GLFW_KEY_A && action == GLFW_PRESS)
    // {
    //     std::cout << "Key: A - Traslacion en X LEFT" << std::endl;
    //     transC.traslacion(vec3(-0.1f, 0.0f, 0.0f), casa);
    //     transE.traslacion(vec3(-0.1f, 0.0f, 0.0f), estrella);
    //     transP.traslacion(vec3(-0.1f, 0.0f, 0.0f), pizza);
    //     updateVertexBuffer();
    // }
    // if (key == GLFW_KEY_D && action == GLFW_PRESS)
    // {
    //     std::cout << "Key: D - Traslacion en X RIGHT" << std::endl;
    //     transC.traslacion(vec3(0.1f, 0.0f, 0.0f), casa);
    //     transE.traslacion(vec3(0.1f, 0.0f, 0.0f), estrella);
    //     transP.traslacion(vec3(0.1f, 0.0f, 0.0f), pizza);
    //     updateVertexBuffer();
    // }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
    {
        camera.rotationAngle -= 90.0f;
    }
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        camera.rotationAngle += 90.0f;
    }
    // if (key == GLFW_KEY_R && action == GLFW_PRESS)
    // {
    //     std::cout << "Key: R - Traslacion inversa" << std::endl;
    //     transC.traslacion_i(casa);
    //     transE.traslacion_i(estrella);
    //     transP.traslacion_i(pizza);
    //     updateVertexBuffer();
    // }



    // // escala
    // if (key == GLFW_KEY_F && action == GLFW_PRESS)
    // {
    //     std::cout << "Key: F - Escala mayor" << std::endl;
    //     transC.escala(vec3(1.25f, 1.25f, 1.25f), casa);
    //     transE.escala(vec3(1.25f, 1.25f, 1.25f), estrella);
    //     transP.escala(vec3(1.25f, 1.25f, 1.25f), pizza);
    //     updateVertexBuffer();
    // }

    // if (key == GLFW_KEY_G && action == GLFW_PRESS)
    // {
    //     std::cout << "Key: G - Escala menor" << std::endl;
    //     transC.escala(vec3(0.75f, 0.75f, 0.75f), casa);
    //     transE.escala(vec3(0.75f, 0.75f, 0.75f), estrella);
    //     transP.escala(vec3(0.75f, 0.75f, 0.75f), pizza);
    //     updateVertexBuffer();
    // }
    
    // if (key == GLFW_KEY_T && action == GLFW_PRESS)
    // {
    //     std::cout << "Key: T - inversa escala" << std::endl;
    //     transC.escala_i(casa);
    //     transE.escala_i(estrella);
    //     transP.escala_i(pizza);
    //     updateVertexBuffer();
    // }

    // // fig original
    // if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    // {
    //     std::cout << "Key: SPACE - Reset fig" << std::endl;
    //     casa.resetFig();
    //     estrella.resetFig();
    //     pizza.resetFig();
    //     updateVertexBuffer();
    // }

    
    // change display mode to lines only
    if(key == GLFW_KEY_R && action == GLFW_PRESS){
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    // change display mode to fill only
    if(key == GLFW_KEY_T && action == GLFW_PRESS){
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    // change display mode to points only 
    if(key == GLFW_KEY_Y && action == GLFW_PRESS){
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    

    // change background color
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        backgroundColor = getRandomColor();}

    // // Switch between shader programs
    // if (key == GLFW_KEY_1 && action == GLFW_PRESS){
    //     currentShaderProgram = 1;
    //     pointColor = getRandomColor();
    // }
    // if (key == GLFW_KEY_2 && action == GLFW_PRESS){
    //     currentShaderProgram = 2;
    //     triangleColor = getRandomColor();
    // }
    // if (key == GLFW_KEY_3 && action == GLFW_PRESS){
    //     currentShaderProgram = 3;
    //     figureColor = getRandomColor();
    // }
    // if (key == GLFW_KEY_4 && action == GLFW_PRESS){
    //     currentShaderProgram = 4;
    //     pointColor = getRandomColor();
    //     triangleColor = getRandomColor();
    //     figureColor = getRandomColor();
    //     starColor = getRandomColor();
    //     pizzaColor = getRandomColor();
    // }
    
    // change square color
    // if (key == GLFW_KEY_C && action == GLFW_PRESS) {
    //     squareColor = getRandomColor();}
    // // change triangle color
    // if (key == GLFW_KEY_V && action == GLFW_PRESS) {
    //     triangleColor = getRandomColor();}
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}