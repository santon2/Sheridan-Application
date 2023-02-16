#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <string>

// This code is the project that I recorded and included in this repository as an mp4. 
// It is a simple creation of 3 "planets" with real ratios used to calculate rotation frequency and skew.


static unsigned int ss_id = 0;
void dump_framebuffer_to_ppm(std::string prefix, unsigned int width, unsigned int height) {
    int pixelChannel = 3;
    int totalPixelSize = pixelChannel * width * height * sizeof(GLubyte);
    GLubyte* pixels = new GLubyte[totalPixelSize];
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    std::string file_name = prefix + std::to_string(ss_id) + ".ppm";
    std::ofstream fout(file_name);
    fout << "P3\n" << width << " " << height << "\n" << 255 << std::endl;
    for (size_t i = 0; i < height; i++)
    {
        for (size_t j = 0; j < width; j++)
        {
            size_t cur = pixelChannel * ((height - i - 1) * width + j);
            fout << (int)pixels[cur] << " " << (int)pixels[cur + 1] << " " << (int)pixels[cur + 2] << " ";
        }
        fout << std::endl;
    }
    ss_id++;
    delete[] pixels;
    fout.flush();
    fout.close();
}

//key board control
void processInput(GLFWwindow* window)
{
    //press escape to exit
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //press p to capture screen
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        std::cout << "Capture Window " << ss_id << std::endl;
        int buffer_width, buffer_height;
        glfwGetFramebufferSize(window, &buffer_width, &buffer_height);
        dump_framebuffer_to_ppm("cube1", buffer_width, buffer_height);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


// shader
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 vertexColor;\n"
"out vec3 objectColor;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0f);\n"
"   objectColor = vertexColor;\n"
"}\0";
const char* fragmentShaderSource = "#version 330 core\n"
"in vec3 objectColor;\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
//"   FragColor = vec4(0.9f, 0.8f, 0.7f, 1.0f);\n" //triangle color green
"   FragColor = vec4(objectColor, 1.0f);\n"
"}\n\0";


float get_sun_rotate_angle_around_itself(float day)
{
    return day * 360.0f / 27.0f;
}
float get_earth_rotate_angle_around_sun(float day)
{
    return day * 360.0f / 365.0f;
}
float get_earth_rotate_angle_around_itself(float day)
{
    return day * 360.0f;
}
float get_moon_rotate_angle_around_earth(float day)
{
    return day * 360.0f / 28.0f;
}
float get_moon_rotate_angle_around_itself(float day)
{
    return day * 360.0f / 28.0f;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    int windowWidth = 1024;
    int windowHeight = 768;
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Assignment1", NULL, NULL);

    if (window == NULL)
    {
        std::cout << "GLFW Window Failed" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "GLAD Initialization Failed" << std::endl;
        return -1;
    }

    //shaders
    int success;
    char error_msg[512];
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vs, 512, NULL, error_msg);
        std::cout << "Vertex Shader Failed: " << error_msg << std::endl;
    }
    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fs, 512, NULL, error_msg);
        std::cout << "Fragment Shader Failed: " << error_msg << std::endl;
    }
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, error_msg);
        std::cout << "Program Link Error: " << error_msg << std::endl;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    /*triangle vertex positions
    float tri_verts[] = {
        -0.8f, -0.8f, 0.0f, //bottom left
         0.8f, -0.8f, 0.0f, //bottom right
         0.0f,  0.8f, 0.0f  //top middle
    };
    */
    
    float day = 365.0f; //rotate_d0.ppm
    //float day = 1.0f; //rotate_d1.ppm
    //float day = 0.3f; //rotate_ddpt3.ppm
    //float day = 128.0f; //rotate_dd128.ppm
    //float day = 365.0f; //rotate_d365.ppm + rotate_d365_earth.ppm
    float increment = 1.0 / 24.0f;
    
    //Cube1 vertex positions
    float cube1_verts[] = {

        //FRONT FACE
        -1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 1.0f, //left bottom front
         1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 1.0f, //right bottom front
        -1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 1.0f, //left top front
         1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 1.0f, //right top front
        -1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 1.0f, //left top front
         1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 1.0f, //right bottom front
 
         //BACK FACE
        -1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, //left bottom back
        -1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f, //left top back
         1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, //right bottom back
         1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f, //right top back
         1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, //right bottom back
        -1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f, //left top back

        //LEFT FACE
        -1.0f, -1.0f,  1.0f, 0.0f, 1.0f, 0.0f, //left bottom front
        -1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f, //left top front
        -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, //left bottom back
        -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 0.0f, //left top back
        -1.0f, -1.0f, -1.0f, 0.0f, 1.0f, 0.0f, //left bottom back
        -1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 0.0f, //left top front
        
         //RIGHT FACE
         1.0f, -1.0f,  1.0f, 1.0f, 1.0f, 0.0f, //right bottom front
         1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, //right bottom back
         1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 0.0f, //right top front
         1.0f,  1.0f, -1.0f, 1.0f, 1.0f, 0.0f, //right top back
         1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 0.0f, //right top front
         1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 0.0f, //right bottom back
        
         //TOP FACE
        -1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 1.0f, //left top front
         1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 1.0f, //right top front
        -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 1.0f, //left top back
         1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 1.0f, //right top back
        -1.0f,  1.0f, -1.0f, 0.0f, 1.0f, 1.0f, //left top back
         1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 1.0f, //right top front

         //BOTTOM FACE
        -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, //left bottom front
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, //left bottom back
         1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, //right bottom back
         1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, //right bottom back
         1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f, //right bottom front
        -1.0f, -1.0f,  1.0f, 0.0f, 0.0f, 1.0f  //left bottom front
   
    };

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f), //Sun Pos
        glm::vec3(20.0f, 0.0f,  0.0f), //Earth Pos
        glm::vec3(30.0f, 0.0f,  0.0f), //Moon Pos
    };

    glm::vec3 cubeScales[] = {
        glm::vec3(4.0f, 4.0f,  4.0f),  //Sun Scale
        glm::vec3(2.5f, 2.5f,  2.5f),  //Earth Scale
        glm::vec3(1.5f, 1.5f,  1.5f),  //Moon Scale
    };

    float tiltAngles[] = {
        0.0f, -23.4f, 0.0f  //Sun, Earth, Moon tilt respectively
    };
    
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube1_verts), cube1_verts, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,6 * sizeof(float), (void*)(3* sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
       
        //TIME STUFF
        day += increment;
        cubePositions[1] = 20.0f * glm::vec3(cos(glm::radians(get_earth_rotate_angle_around_sun(day))), 0, -sin(glm::radians(get_earth_rotate_angle_around_sun(day)))); //Earth Pos update
        cubePositions[2] = 10.0f * glm::vec3(cos(glm::radians(get_moon_rotate_angle_around_earth(day))), 0, -sin(glm::radians(get_moon_rotate_angle_around_earth(day)))) + cubePositions[1]; //Moon Pos update


        //background color
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.3f, 0.4f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //draw things
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);


        //glm::mat4 view = glm::lookAt(glm::vec3(3.0f, 4.0f, 5.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); cube1.ppm
        //glm::mat4 view = glm::lookAt(glm::vec3(-6.0f, -7.0f, -8.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); cube2.ppm


        for (unsigned int i = 0; i < 3; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            //glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 100.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); //static_front.ppm
            //glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 100.0f, 1.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); //static_top.ppm
            //glm::mat4 view = glm::lookAt(glm::vec3(100.0f, 1.0f, 1.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); //static_right.ppm
            //glm::mat4 view = glm::lookAt(glm::vec3(60.0f, 70.0f, 80.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); //static_sun.ppm
            //glm::mat4 view = glm::lookAt(glm::vec3(-60.0f, 50.0f, 40.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); //static_earth.ppm
            //glm::mat4 view = glm::lookAt(glm::vec3(50.0f, 60.0f, 70.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); //rotate_d0.ppm + the rest of task 3 except 3.6
            //glm::mat4 view = glm::lookAt(glm::vec3(-30.0f, -40.0f, -50.0f), cubePositions[1], glm::vec3(0, 1, 0)); //rotate_d365_earth.ppm
            //glm::mat4 view = glm::lookAt(glm::vec3(50.0f, 50.0f, 100.0f), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); //dynamic sun
            //glm::mat4 view = glm::lookAt(glm::vec3(50.0f, 50.0f, 100.0f), cubePositions[1], glm::vec3(0, 1, 0)); //dynamic earth
            glm::mat4 view = glm::lookAt(glm::vec3(50.0f, 50.0f, 100.0f), cubePositions[2], glm::vec3(0, 1, 0)); //dynamic moon
            glm::mat4 projection = glm::perspective(glm::radians(30.0f), 4.0f / 3.0f, 0.1f, 1000.0f);
           
            
            model = glm::translate(model, cubePositions[i]);
            model = glm::scale(model, cubeScales[i]);
            model = glm::rotate(model, glm::radians(tiltAngles[i]), glm::vec3(0.0f, 0.0f, 1.0f));

            if (i == 0) { //sun
                model = glm::rotate(model, glm::radians(get_sun_rotate_angle_around_itself(day)), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            else if (i == 1) { //earth
                model = glm::rotate(model, glm::radians(get_earth_rotate_angle_around_itself(day)), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            else if (i == 2) { //moon
                model = glm::rotate(model, glm::radians(get_moon_rotate_angle_around_itself(day)), glm::vec3(0.0f, 1.0f, 0.0f));

            }


            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    //release resource
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}
