#define GLAD_GL_IMPLEMENTATION
#include <SFML/Window.hpp>
#include <glad/gl.h>

#include <iostream>
#include <optional>

#include "../include/hotshaders.hh"
#include "Mesh.hh"

struct CorpoCeleste{
    std::string nome;
    float raggio;             // dimensione
    float dist_orbita;        // distanza dal Sole (0 per il Sole)
    float vel_orbita;         // rad/s di rivoluzione attorno al Sole
    float vel_rotazione;      // rad/s di rotazione su se stesso
    float inclinazione_deg;   // inclinazione assiale
    glm::vec3 colore;
    bool is_sole = false;
    glm::mat4 model(float t) const {
        //scala in base al raggio del corpo
        glm::mat4 scala(
            raggio, 0.0f,   0.0f,   0.0f,   // 1a colonna
            0.0f,   raggio, 0.0f,   0.0f,   // 2a colonna
            0.0f,   0.0f,   raggio, 0.0f,   // 3a colonna
            0.0f,   0.0f,   0.0f,   1.0f    // 4a colonna
        );
        //rotazione attorno asse y proprio 
        float ar = vel_rotazione * t;
        float rs = glm::sin(ar);
        float rc = glm::cos(ar);
        glm::mat4 spin(
             rc,  0.0f,  -rs,  0.0f,   // 1a colonna
            0.0f, 1.0f, 0.0f, 0.0f,   // 2a colonna
             rs,  0.0f,   rc,  0.0f,   // 3a colonna
            0.0f, 0.0f, 0.0f, 1.0f    // 4a colonna
        );
        //inclinazione asso attorno a Z
        float ts = glm::sin(glm::radians(inclinazione_deg));
        float tc = glm::cos(glm::radians(inclinazione_deg));
        glm::mat4 tilt(
              tc,   ts,  0.0f, 0.0f,   // 1a colonna
             -ts,   tc,  0.0f, 0.0f,   // 2a colonna
            0.0f, 0.0f, 1.0f, 0.0f,   // 3a colonna
            0.0f, 0.0f, 0.0f, 1.0f    // 4a colonna
        );
        //posizione orbita (traslazione)
        float ao = vel_orbita * t;
        float px =  dist_orbita * glm::cos(ao);
        float pz = -dist_orbita * glm::sin(ao);
        glm::mat4 orbita(
            1.0f, 0.0f, 0.0f, 0.0f,   // 1a colonna
            0.0f, 1.0f, 0.0f, 0.0f,   // 2a colonna
            0.0f, 0.0f, 1.0f, 0.0f,   // 3a colonna
              px, 0.0f,   pz,  1.0f    // 4a colonna: qui vive la traslazione
        );

        return orbita * tilt * spin * scala;

    }
}; 

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
        Mesh sole=Mesh::crea_sfera(40, 20, 0.5f );
        sole.pack4gpu(points, indices);
        send_arrays_2a3f();
    }
    ~Scene () { clean (); }
    void clean(){
        glDeleteVertexArrays (1, &vao);
        glDeleteBuffers (1, &vbo);
        glDeleteBuffers (1, &ebo);
    }
    void draw(){
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

class Camera{
    private:
        float phi_deg = 210.0f;
        float theta_deg = 20.0f;
        // distanza della camera dal centro della scena
        float distance = 6.0f;
        // parametro della proiezione
        float focal_distance = 4.0f;
        GLint vp_loc;
    public:
    Camera(Shaders& shaders){
        vp_loc = glGetUniformLocation(shaders.program, "vp");
        update(); 
    }
    //ruotare camera intorno alla scena 
    void drag (float dx, float dy)
    {
        phi_deg += dx * 0.1;
        theta_deg += dy * 0.1;
        theta_deg = theta_deg > 90.0? 90.0 : theta_deg;
        theta_deg = theta_deg < -90.0? -90.0 : theta_deg;
        update ();
    }

    void zoom(float dy)
    {
        focal_distance += dy * 0.05f;
        if (focal_distance < 0.5f)
            focal_distance = 0.5f;
        update();
    }

    private:
    void update(){
        //rotazione attorno ad Y
        float phi = glm::radians(phi_deg);
        float ps = glm::sin(phi);
        float pc = glm::cos(phi);
        glm::mat4 ry = glm::mat4(
                        pc, 0.0, -ps, 0.0,
                        0.0, 1.0, 0.0, 0.0,
                        ps, 0.0,  pc, 0.0,
                        0.0, 0.0, 0.0, 1.0
                        );
        //rotazione attorno ad x
        float ts = glm::sin (glm::radians (theta_deg));
        float tc = glm::cos (glm::radians (theta_deg));
        glm::mat4 rx = glm::mat4(
                        1.0, 0.0, 0.0, 0.0, 
                        0.0,  tc, ts,  0.0, 
                        0.0, -ts, tc,  0.0, 
                        0.0, 0.0, 0.0, 1.0
                        );
        //traslazione
        glm::mat4 tz = glm::mat4(
                    1.0, 0.0, 0.0, 0.0, 
                    0.0, 1.0, 0.0, 0.0, 
                    0.0, 0.0, 1.0, 0.0,
                    0.0, 0.0, -distance, 1.0 
                    );

        //proiezione
        const float fcp = 100.0;
        const float ncp = 1.0;
        float a = -(fcp + ncp) / (fcp - ncp);
        float b = -2.0 * fcp * ncp / (fcp - ncp);
        glm::mat4 projection(
            focal_distance, 0.0, 0.0, 0.0,
            0.0, focal_distance, 0.0, 0.0,
            0.0, 0.0, a, -1.0,
            0.0, 0.0, b, 0.0
        );

        //VP
        glm::mat4 vp = projection * tz * rx * ry;
        // manda VP alla GPU
        glUniformMatrix4fv(
            vp_loc,
            1,
            GL_FALSE,
            &vp[0][0]
        );

    }
};


int main(){
    Setup setup;
    sf::Window& window = *setup.window;
    Shaders shaders("../Tappa02/02_vertex.vert",
                "../Tappa02/02_fragment.frag");
    shaders.use();
    GLint objectColorLoc = glGetUniformLocation(shaders.program, "objectColor");
    GLint sunPositionLoc = glGetUniformLocation(shaders.program, "sunPosition");
    GLint isSunLoc       = glGetUniformLocation(shaders.program, "isSun");
    GLint modelLoc       = glGetUniformLocation(shaders.program, "model");

    Camera camera(shaders);
    Scene scene;
    glEnable(GL_DEPTH_TEST);
    
    // la luce parte dall'origine e non si muove mai: una volta sola
    glm::vec3 sunPosition(0.0f, 0.0f, 0.0f);
    glUniform3fv(sunPositionLoc, 1, &sunPosition[0]);

    std::vector<CorpoCeleste> sistema = {
    // nome    raggio dist  v_orb  v_rot  incl    colore              sole
    { "Sole",  1.0f,  0.0f, 0.0f,  0.5f,  7.25f, {1.0f, 0.6f, 0.05f}, true }, 
    };

    sf::Clock clock;        
    
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
            else if(const auto* mouse = event->getIf<sf::Event::MouseMoved>()){
                static float old_x = mouse->position.x;
                static float old_y = mouse->position.y;
                float dx = mouse->position.x - old_x;
                float dy = mouse->position.y - old_y;
                old_x = mouse->position.x;
                old_y = mouse->position.y;
                if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) camera.drag(dx, dy);
                else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) camera.zoom(dy);
            }

        }

        float t = clock.getElapsedTime().asSeconds();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const CorpoCeleste& corpo : sistema) {
            glm::mat4 model = corpo.model(t);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
            glUniform3fv(objectColorLoc, 1, &corpo.colore[0]);
            glUniform1i(isSunLoc, corpo.is_sole ? 1 : 0);
            scene.draw();
        }
        window.display();
    }

    return 0;
}
