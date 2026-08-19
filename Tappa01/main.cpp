#define GLAD_GL_IMPLEMENTATION
#include <SFML/Window.hpp>
#include <glad/gl.h>

#include <iostream>
#include <optional>

#include "../include/hotshaders.hh"
#include "Mesh.hh"

class Setup{
    public: sf::Window* window;
    Setup(){
        sf::ContextSettings settings;
        settings.depthBits = 32;
        settings.stencilBits = 8;
        settings.antiAliasingLevel = 4;
        settings.attributeFlags = sf::ContextSettings::Attribute::Core;
        settings.majorVersion = 4;
        settings.minorVersion = 1;
        window = new sf::Window(
            sf::VideoMode({800, 600}),
            "Solar System 3D",
            sf::Style::Default,
            sf::State::Windowed,
            settings
        );
        window->setVerticalSyncEnabled(true);

        if (!window->setActive(true)){
            std::cerr << "Errore durante l'attivazione del contesto OpenGL."
                    << std::endl;
            exit(1);
        }
        sf::ContextSettings gotten = window->getSettings();
        std::cout << "depth bits: " << gotten.depthBits << std::endl;
        std::cout << "stencil bits: " << gotten.stencilBits << std::endl;
        std::cout << "antialiasing level: " << gotten.antiAliasingLevel << std::endl;
        std::cout << "SFML GL version: " << gotten.majorVersion << "." << gotten.minorVersion << std::endl;

        int version = gladLoadGL (sf::Context::getFunction);
        if (!version) {
            std::cerr << "Failure: error during glad loading." << std::endl;
            exit(1);
        }
        std::cout << "GLAD GL version: "<<GLAD_VERSION_MAJOR(version)<<"."<< GLAD_VERSION_MINOR(version)<<std::endl;
    }
    ~Setup ()
    {
        delete window;
    }
};

class Scene{
    private:
        std::vector<float> points;
        std::vector<unsigned int> indices;
        GLuint vbo;
        GLuint ebo;
        GLuint vao;
    
    public:
    Scene(){
        Mesh sphere=Mesh::crea_sfera(40, 20, 0.5f );
        sphere.pack4gpu(points, indices);
        send_arrays_2a3f();
    }
    ~Scene () { clean (); }
    void clean(){
        glDeleteVertexArrays (1, &vao);
        glDeleteBuffers (1, &vbo);
        glDeleteBuffers (1, &ebo);
    }
    void draw(){
        glClear( 
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
        );
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, //disegna usando indici nell'EBO
            indices.size(),
            GL_UNSIGNED_INT, //gli indici sono int 
            0 //offset iniziale nell'EBO
        );
    }
    private:
    void send_arrays_2a3f(){
        glGenBuffers (1, &vbo); //creo vbo
        glBindBuffer (GL_ARRAY_BUFFER, vbo); //seleziono VBO
        glBufferData (GL_ARRAY_BUFFER, //copiamo points nel VBO
                      points.size () * sizeof (float),
                      points.data (),
                      GL_STATIC_DRAW);
        glGenVertexArrays (1, &vao); //creo vao
        glBindVertexArray (vao);
        glVertexAttribPointer (0, //nel vbo il primo attributo è la pos
                               3, 
                               GL_FLOAT,
                               GL_FALSE,
                               6 * sizeof(float), //per il vertice successivo si saltano 6 float
                               (void*)0);
        glEnableVertexAttribArray (0);
        glVertexAttribPointer (1,
                               3,
                               GL_FLOAT,
                               GL_FALSE,
                               6 * sizeof(float),
                               (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray (1);
        glGenBuffers(1, &ebo); 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo); //buffer per indici
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     indices.size () * sizeof (unsigned int),
                     indices.data (),
                     GL_STATIC_DRAW);
    }
};

int main(){
    Setup setup;
    sf::Window& window = *setup.window;
    Shaders shaders("../Tappa01/01_vertex.vert",
                "../Tappa01/01_fragment.frag");
    shaders.use();

    Scene scene;
    glEnable(GL_DEPTH_TEST);
    
    
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent()){
            if (event->is<sf::Event::Closed>()){
                window.close();
            }
            else if(const auto* resized= event->getIf<sf::Event::Resized>() ){
                glViewport(
                    0,
                    0,
                    resized->size.x,
                    resized->size.y
                );
            }
        }

        scene.draw();
        window.display();
    }

    return 0;
}