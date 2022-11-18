#ifndef RENDERER_H_
#define RENDERER_H_

#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "common.hpp"
#include "timer.hpp"
#include "solver.hpp"
#include "OGL/shader.hpp"
#include "OGL/tetrahedron_mesh.hpp"
#include "OGL/ground_mesh.hpp"

int default_src_width  = 1600;
int default_src_height = 1000;

float refresh_interval = (1/30);

// View parameters
float theta = 0.0;
float phi = 0.0;
float camradius = 5.0;
float cameraspeed = 0.5;

float camX = camradius;
float camY = 0.0;
float camZ = 0.0;

// Allow window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

// Keyboard input: JKIL for camera motion (also escape to close window)
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    // J key will rotate left
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        theta -= cameraspeed;
        if (theta < 0.0) theta += 360.0;
        camX = camradius * cos(glm::radians(phi)) * cos(glm::radians(theta));
        camY = camradius * cos(glm::radians(phi)) * sin(glm::radians(theta));
        camZ = camradius * sin(glm::radians(phi));
    }
    // L key will rotate right
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        theta += cameraspeed;
        if (theta >= 360.0) theta -= 360.0;
        camX = camradius * cos(glm::radians(phi)) * cos(glm::radians(theta));
        camY = camradius * cos(glm::radians(phi)) * sin(glm::radians(theta));
        camZ = camradius * sin(glm::radians(phi));
    }
    // I key will rotate right
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        if (phi < 90.0-cameraspeed) phi += cameraspeed;
        camX = camradius * cos(glm::radians(phi)) * cos(glm::radians(theta));
        camY = camradius * cos(glm::radians(phi)) * sin(glm::radians(theta));
        camZ = camradius * sin(glm::radians(phi));
    }
    // K key will rotate right
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        if (phi > -90.0+cameraspeed) phi -= cameraspeed;
        camX = camradius * cos(glm::radians(phi)) * cos(glm::radians(theta));
        camY = camradius * cos(glm::radians(phi)) * sin(glm::radians(theta));
        camZ = camradius * sin(glm::radians(phi));
    }
}


class Renderer
{
private:
    GLFWwindow* window;

    GLuint shaderProgram;
    GLuint fragmentShader;
    GLuint vertexShader;

    GLuint modelLoc;
    GLuint viewLoc;
    GLuint projectionLoc;

    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;

    GLuint ground_vao;          
    GLuint ground_mesh_vbo;    
    GLuint ground_mesh_ibo;     
    GLuint ground_mesh_color;

    GLuint tetrahedron_vao;
    GLuint tetrahedron_vbo;

    GLuint hexahedron_vao;
    GLuint hexahedron_vbo;
    GLuint hexahedron_ibo;
    GLuint hexahedron_color;

    Timer timer;
    Solver sovler;

public:
    Renderer();
    ~Renderer();

    void initialize()
    {
        timer.reset(refresh_interval);
        GLObj::build_ground_mesh();
      
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(default_src_width, default_src_height, k_project_name, NULL, NULL);
        if (window == NULL) {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(window);
    
        // GLAD manages function pointers for OpenGL, so we want to initialize GLAD before we call any OpenGL function
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return;
        }
        glViewport(0, 0, default_src_width, default_src_height);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // Enable depth buffering, backface culling
        glEnable(GL_DEPTH_TEST);
        // glEnable(GL_CULL_FACE);
        // glCullFace(GL_BACK);

        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glUseProgram(shaderProgram);

        // tetrahedron
        glGenVertexArrays(1, &tetrahedron_vao);
        glBindVertexArray(tetrahedron_vao);
            glGenBuffers(1, &tetrahedron_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, tetrahedron_vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLObj::tetrahedron), GLObj::tetrahedron, GL_STATIC_DRAW);
        
        // hexahedron
        glGenVertexArrays(1, &hexahedron_vao);
        glBindVertexArray(hexahedron_vao);
            glGenBuffers(1, &hexahedron_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, hexahedron_vbo);
            glBufferData(GL_ARRAY_BUFFER, sovler.get_hexahedron_vertices().size()*sizeof(glm::vec3), glm::value_ptr(sovler.get_hexahedron_vertices()[0]), GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

            glGenBuffers(1, &hexahedron_ibo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hexahedron_ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sovler.get_hexahedron_indices().size() * sizeof(unsigned int), &sovler.get_hexahedron_indices()[0], GL_STATIC_DRAW);

        // ground mesh
        glGenVertexArrays(1, &ground_vao);
        glBindVertexArray(ground_vao);
            glGenBuffers(1, &ground_mesh_vbo);
            glBindBuffer(GL_ARRAY_BUFFER, ground_mesh_vbo);
            glBufferData(GL_ARRAY_BUFFER, GLObj::ground_mesh_vertices.size()*sizeof(glm::vec3), glm::value_ptr(GLObj::ground_mesh_vertices[0]), GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                
            glGenBuffers(1, &ground_mesh_ibo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ground_mesh_ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, GLObj::ground_mesh_indices.size()*sizeof(glm::uvec4), glm::value_ptr(GLObj::ground_mesh_indices[0]), GL_STATIC_DRAW);

        // Declare model/view/projection matrices
        model = glm::mat4(1.0f);
        modelLoc = glGetUniformLocation(shaderProgram, "model");
        view = glm::mat4(1.0f);
        viewLoc = glGetUniformLocation(shaderProgram, "view");
        projection = glm::mat4(1.0f);
        projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        // Set Projection matrix
        projection = glm::perspective(glm::radians(45.0f), 1.0f*default_src_width/default_src_height, 0.1f, 100.0f);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    }

    void start_looping()
    {
        while(!glfwWindowShouldClose(window) && !timer.is_time_to_stop())
        {
            processInput(window);
            glfwPollEvents();

            if (timer.is_time_to_draw()) 
            {
                timer.update_next_display_time();
                glBindBuffer(GL_ARRAY_BUFFER, hexahedron_vbo);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sovler.get_hexahedron_vertices().size()*sizeof(glm::vec3), glm::value_ptr(sovler.get_hexahedron_vertices()[0]));
                draw();
            }
            sovler.compute_next_state();
            timer.update_simulation_time();
        }
        delete_GLBuffers();
        glfwTerminate();
    }

    void draw() 
    {
        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glEnable(GL_DEPTH_TEST);

        // Set view matrix
        view = glm::lookAt(glm::vec3(camX, camY, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // render ground mesh
        glBindVertexArray(ground_vao);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ground_mesh_ibo);
            glDrawElements(GL_LINES, GLObj::ground_mesh_indices.size()*4, GL_UNSIGNED_INT, (void*)0);

        // render hexaedron
        glBindVertexArray(hexahedron_vao);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hexahedron_ibo);
            glDrawElements(GL_TRIANGLES, sovler.get_hexahedron_indices().size(), GL_UNSIGNED_INT, (void*)0);

        // render tetrahedron
        glBindVertexArray(tetrahedron_vao);
            glBindBuffer(GL_ARRAY_BUFFER, tetrahedron_vbo);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);   
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            model = glm::mat4(1.0f);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);

        // glDisable(GL_DEPTH_TEST);
        glfwSwapBuffers(window);
    }

    void delete_GLBuffers()
    {
        glDeleteVertexArrays(1, &tetrahedron_vao);
        glDeleteBuffers(1, &tetrahedron_vbo);
        glDeleteBuffers(1, &ground_mesh_vbo);
        glDeleteBuffers(1, &ground_mesh_ibo);
        glDeleteProgram(shaderProgram);
    }

};

Renderer::Renderer() {};
Renderer::~Renderer() {};

#endif // RENDERER_H_