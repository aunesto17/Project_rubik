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
#include <filesystem>
#include <fstream> 

#include "vertex.h"
#include "matriz.h"
#include "rubik.h"
//#include "transform.h"
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
    //"layout (location = 2) in vec3 aTexCoord;\n"
    "layout (location = 2) in vec2 aTexCoord;\n"

    "out vec3 ourColor;\n"
    "out vec2 TexCoord;\n"

    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"

    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
    //"   TexCoord = vec2(aTexCoord.x,aTexCoord.y);\n"    
    "   TexCoord = vec2(aTexCoord.x,aTexCoord.y);\n"  
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
    "           texColor = texture(texture1, TexCoord);\n"
    "           break;\n"
    "       case 1:  \n"
    "           texColor = texture(texture2, TexCoord);\n"
    "           break;\n"
    "       case 2:  \n"
    "           texColor = texture(texture3, TexCoord);\n"
    "           break;\n"
    "       case 3:  \n"
    "           texColor = texture(texture4, TexCoord);\n"
    "           break;\n"
    "       case 4:  \n"
    "           texColor = texture(texture5, TexCoord);\n"
    "           break;\n"
    "       case 5:  \n"
    "           texColor = texture(texture5, TexCoord);\n"
    "           break;\n"
    "       default: \n"
    "           texColor = vec4(1.0);\n"
    "   }\n"
    
    //"   FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0f);\n"
    "   FragColor = texColor * vec4(ourColor, 1.0f);\n"     
    "}\0";

// Function to load a texture and handle errors
unsigned int loadTexture(const char* path) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image data
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
    
    if (data) {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        std::cout << "Texture loaded successfully: " << path 
                  << " (" << width << "x" << height 
                  << ", " << nrChannels << " channels)" << std::endl;
    }
    else {
        std::cout << "Failed to load texture: " << path << std::endl;
        std::cout << "STB Error: " << stbi_failure_reason() << std::endl;
    }

    stbi_image_free(data);
    return texture;
}

// Also check if the files exist before trying to load them
void checkTextureFiles() {
    const char* textureFiles[] = {
        "letter-u.png",
        "letter-c.png",
        "letter-s.png",
        "letter-p.png",
        "ucsp-logo.png"
    };

    for (const char* file : textureFiles) {
        std::ifstream f(file);
        if (!f.good()) {
            std::cout << "Texture file not found: " << file << std::endl;
        } else {
            std::cout << "Found texture file: " << file << std::endl;
        }
    }
}

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

    // ---------------------------------------
    // texturas
    // ---------------------------------------
    unsigned int textureU, textureC, textureS, textureP, textureUL;
    // glGenTextures(cuantas texturas queremos generar, las guarda en este array(en este caso solo 1));
    // glGenTextures(1, &textureU);
    // // bind the texture
    // glBindTexture(GL_TEXTURE_2D, textureU);
    // // seteamos las opciones de envolvimiento/filtro de la textura
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // // load and generate the texture
    // int width, height, nrChannels;
    // stbi_set_flip_vertically_on_load(true);
    // unsigned char *data = stbi_load("letter-u.png", &width, &height, &nrChannels, 0);
    // // set the texture wrapping parameters
    // //  1. texture target al objeto bindeado en el paso anterior
    // //  2. nivel de mipmap (0 por defecto)
    // //  3. formato para guardar la textura(la imagen es RGB entonces usamos GL_RGB)
    // //  4. ancho de la textura (usamos los valores ya obtenidos)
    // //  5. alto de la textura
    // //  6. siempre 0
    // //  7. formato de la imagen (RGB)
    // //  8. tipo de dato de la imagen (unsigned byte)
    // //  9. la imagen en si
    // if(data){
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //     glGenerateMipmap(GL_TEXTURE_2D);
    // } else {
    //     std::cout << "fallo en cargar textura U" << std::endl;
    // }
    // // liberar la memoria de la imagen
    // stbi_image_free(data);


    // // textura C
    // glGenTextures(1, &textureC);
    // glBindTexture(GL_TEXTURE_2D, textureC);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // data = stbi_load("letter-c.png", &width, &height, &nrChannels, 0);
    // if(data){
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //     glGenerateMipmap(GL_TEXTURE_2D);
    // } else {
    //     std::cout << "fallo en cargar textura C" << std::endl;
    // }
    // stbi_image_free(data);

    // // textura S
    // glGenTextures(1, &textureS);
    // glBindTexture(GL_TEXTURE_2D, textureS);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // data = stbi_load("letter-s.png", &width, &height, &nrChannels, 0);
    // if(data){
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //     glGenerateMipmap(GL_TEXTURE_2D);
    // } else {
    //     std::cout << "fallo en cargar textura S" << std::endl;
    // }
    // stbi_image_free(data);

    // // textura P
    // glGenTextures(1, &textureP);
    // glBindTexture(GL_TEXTURE_2D, textureP);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // data = stbi_load("letter-p.png", &width, &height, &nrChannels, 0);
    // if(data){
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //     glGenerateMipmap(GL_TEXTURE_2D);
    // } else {
    //     std::cout << "fallo en cargar textura P" << std::endl;
    // }
    // stbi_image_free(data);

    // // textura logo UCSP
    // glGenTextures(1, &textureUL);
    // glBindTexture(GL_TEXTURE_2D, textureUL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // data = stbi_load("ucsp-logo.png", &width, &height, &nrChannels, 0);
    // if(data){
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //     glGenerateMipmap(GL_TEXTURE_2D);
    // } else {
    //     std::cout << "fallo en cargar textura UL" << std::endl;
    // }

    checkTextureFiles();

    // Load each texture and check for errors
    // textureU = loadTexture("letter-u.png");
    // textureC = loadTexture("letter-c.png");
    // textureS = loadTexture("letter-s.png");
    // textureP = loadTexture("letter-p.png");
    // textureUL = loadTexture("ucsp-logo.png");

    textureU = loadTexture("container.jpg");
    textureC = loadTexture("container.jpg");
    textureS = loadTexture("container.jpg");
    textureP = loadTexture("container.jpg");
    textureUL = loadTexture("container.jpg");

    // textureU = loadTexture("letter-u.png");
    // textureC = loadTexture("letter-u.png");
    // textureS = loadTexture("letter-u.png");
    // textureP = loadTexture("letter-u.png");
    // textureUL = loadTexture("letter-u.png");

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

    CuboRubik cuboRubik;

    // tell opengl for each sampler to which texture it belongs
    //glUseProgram(shaderProgram);
    //glUniform1i(glGetUniformLocation(shaderProgram, "ourTexture"), 0);
    int texture1Loc = glGetUniformLocation(shaderProgram, "texture1");
    int texture2Loc = glGetUniformLocation(shaderProgram, "texture2");
    int texture3Loc = glGetUniformLocation(shaderProgram, "texture3");
    int texture4Loc = glGetUniformLocation(shaderProgram, "texture4");
    int texture5Loc = glGetUniformLocation(shaderProgram, "texture5");
    int faceIndexLoc = glGetUniformLocation(shaderProgram, "faceIndex");
    
    
    // glGenVertexArrays(1, &VAO);
	// // glGenVertexArrays(26, VAOs);
    // glGenBuffers(1, &VBO);
	// // glGenBuffers(26, VBOs);
    // // glGenBuffers(26, EBOs);

    // // cubos
    // glBindVertexArray(VAOs[0]);
	// glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    // glBufferData(GL_ARRAY_BUFFER, cubo.vertices.size() * sizeof(float), &cubo.vertices[0], GL_STATIC_DRAW);
    // // EBOs
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubo.indices.size() * sizeof(float), std::data(cubo.indices), GL_STATIC_DRAW);
    // // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // // position attribute
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);
    // // color attribute
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    // glEnableVertexAttribArray(1);
    // // texture coord attribute
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	// glEnableVertexAttribArray(2);


    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    //glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPointSize(10.f);
    glLineWidth(5.f);
    

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        //processInput(window);

        glfwSetKeyCallback(window, key_callback);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // render
        // ------
		// color del background
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // naranja
		glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);
        

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
        //float aspect = width / height;
        float aspect = SCR_WIDTH / SCR_HEIGHT;
        float size = 4.0f;  // Adjust this to control zoom level
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

        // Bind textures on corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureU);  // Top/Bottom texture
        glUniform1i(texture1Loc, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureC);   // Left texture
        glUniform1i(texture2Loc, 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textureS);   // Front texture
        glUniform1i(texture3Loc, 2);

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, textureP);   // Right texture
        glUniform1i(texture4Loc, 3);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, textureUL);   // Back texture
        glUniform1i(texture5Loc, 4);
        

        //glUniform3f(colorLoc, pointColor.x, pointColor.y, pointColor.z);
        // glBindVertexArray(VAOs[0]);
        // glDrawArrays(GL_TRIANGLES, 0, 36);
        cuboRubik.draw(shaderProgram);

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
        camera.rotationAngle -= 55.0f;
    }
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        camera.rotationAngle += 55.0f;
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