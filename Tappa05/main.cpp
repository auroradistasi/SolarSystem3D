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
    bool is_sun = false;
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
             rc,  0.0f,  -rs,  0.0f, 
            0.0f, 1.0f, 0.0f, 0.0f,   
             rs,  0.0f,   rc,  0.0f,   
            0.0f, 0.0f, 0.0f, 1.0f    
        );
        //inclinazione asse 
        float ts = glm::sin(glm::radians(inclinazione_deg));
        float tc = glm::cos(glm::radians(inclinazione_deg));
        glm::mat4 tilt(
              tc,   ts,  0.0f, 0.0f,   
             -ts,   tc,  0.0f, 0.0f,   
            0.0f, 0.0f, 1.0f, 0.0f,   
            0.0f, 0.0f, 0.0f, 1.0f    
        );
        //posizione orbita (traslazione)
        float ao = vel_orbita * t;
        float px =  dist_orbita * glm::cos(ao);
        float pz = -dist_orbita * glm::sin(ao);
        glm::mat4 orbita(
            1.0f, 0.0f, 0.0f, 0.0f,  
            0.0f, 1.0f, 0.0f, 0.0f,   
            0.0f, 0.0f, 1.0f, 0.0f,   
              px, 0.0f,   pz,  1.0f   
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
        Mesh sfera=Mesh::crea_sfera(40, 20, 0.5f );
        sfera.pack4gpu(points, indices);
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
        glVertexAttribPointer (0,
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
class Orbita{
    private:
        static const unsigned int seg=128;
        GLuint vbo;
        GLuint vao;
        std::vector<float> punti;
    public:
    Orbita(){
        crea_cerchio();
        send_arrays_1a3f();
    }

    ~Orbita(){ clean(); }
    void clean(){
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
    }

    void draw(){
        glBindVertexArray(vao);
        glDrawArrays(GL_LINE_LOOP, //collega i vertici a catena e chiude l'anello
            0, //primo vertice
            seg  //quanti vertici
        );
    }

    private:
    void crea_cerchio(){
        punti.clear();
        for (unsigned int i = 0; i < seg; ++i) {
            float a = 2.0f * (float)M_PI * i / seg; //calcolo angolo per punto i
            punti.push_back( cosf(a));   // x
            punti.push_back( 0.0f);      // y è sempre 0
            punti.push_back(-sinf(a));   // z
        }
    }

    void send_arrays_1a3f(){
        glGenVertexArrays (1, &vao);
        glBindVertexArray (vao);
        glGenBuffers (1, &vbo);
        glBindBuffer (GL_ARRAY_BUFFER, vbo);
        glBufferData (GL_ARRAY_BUFFER,
                      punti.size () * sizeof (float),
                      punti.data (),
                      GL_STATIC_DRAW);
        glVertexAttribPointer (0,
                               3,
                               GL_FLOAT,
                               GL_FALSE,
                               3 * sizeof(float),
                               (void*)0);
        glEnableVertexAttribArray (0);
    }
};


class Camera{
    private:
        float phi_deg = 210.0f;
        float theta_deg = 20.0f;
        // distanza della camera dal centro della scena
        float distance = 60.0f;
        // parametro della proiezione
        float focal_distance = 2.0f;
        float aspect = 4.0f / 3.0f;
        GLint vp_loc;
        glm::vec3 target = glm::vec3(0.0f); 
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
        distance += dy * distance / 100.0f;
        if (distance < 0.5f)
            distance = 0.5f;
        if (distance > 140.0f) distance = 140.0f;
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
        const float fcp = distance + 200.0f;
        const float ncp = distance * 0.01f;
        float a = -(fcp + ncp) / (fcp - ncp);
        float b = -2.0 * fcp * ncp / (fcp - ncp);
        glm::mat4 projection(
            focal_distance/aspect, 0.0, 0.0, 0.0,
            0.0, focal_distance, 0.0, 0.0,
            0.0, 0.0, a, -1.0,
            0.0, 0.0, b, 0.0
        );
        glm::mat4 tt(
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            -target.x, -target.y, -target.z, 1.0f
        );
        //VP
        glm::mat4 vp = projection * tz * rx * ry *tt;
        // manda VP alla GPU
        glUniformMatrix4fv(
            vp_loc,
            1,
            GL_FALSE,
            &vp[0][0]
        );
        

    }
    public:
    void set_aspect(float w, float h){
        if (h > 0.0f) aspect = w / h;
        else aspect = 1.0f;
        update();
    }
    void set_target(const glm::vec3& p) { target = p; update(); }
    void set_distance(float d) { distance = d; update(); }
};


int main(){
    Setup setup;
    sf::Window& window = *setup.window;
    Shaders shaders("../Tappa05/05_vertex.vert",
                "../Tappa05/05_fragment.frag");
    shaders.use();
    GLint objectColorLoc = glGetUniformLocation(shaders.program, "objectColor");
    GLint sunPositionLoc = glGetUniformLocation(shaders.program, "sunPosition");
    GLint tipoLoc = glGetUniformLocation(shaders.program, "tipo");
    GLint modelLoc = glGetUniformLocation(shaders.program, "model");

    Camera camera(shaders);
    camera.set_aspect(static_cast<float>(window.getSize().x),
                      static_cast<float>(window.getSize().y));
    Scene scene;
    Orbita orbita;
    glm::vec3 coloreOrbita(0.11f, 0.13f, 0.17f);
    glEnable(GL_DEPTH_TEST);
    
    // la luce parte dall'origine e non si muove mai: una volta sola
    glm::vec3 sunPosition(0.0f, 0.0f, 0.0f);
    glUniform3fv(sunPositionLoc, 1, &sunPosition[0]);

    std::vector<CorpoCeleste> sistema = {
    // nome        raggio  dist    v_orb    v_rot   incl      colore            sole
    { "Sole",      5.00f,   0.0f, 0.000f,  0.30f,   7.25f, {1.00f,0.45f,0.02f}, true  },
    { "Mercurio",  0.38f,   6.8f, 0.520f,  0.20f,   0.03f, {0.62f,0.58f,0.54f}, false },
    { "Venere",    0.95f,   8.1f, 0.370f,  0.10f, 177.36f, {0.90f,0.78f,0.50f}, false },
    { "Terra",     1.00f,   9.0f, 0.310f,  1.57f,  23.44f, {0.20f,0.42f,0.85f}, false },
    { "Marte",     0.53f,  10.4f, 0.250f,  1.57f,  25.19f, {0.78f,0.33f,0.20f}, false },
    { "Giove",     3.66f,  15.8f, 0.140f,  2.42f,   3.13f, {0.80f,0.68f,0.52f}, false },
    { "Saturno",   3.05f,  19.2f, 0.100f,  2.33f,  26.73f, {0.88f,0.80f,0.60f}, false },
    { "Urano",     1.99f,  24.1f, 0.072f,  1.85f,  97.77f, {0.55f,0.80f,0.83f}, false },
    { "Nettuno",   1.93f,  27.9f, 0.057f,  1.90f,  28.32f, {0.22f,0.35f,0.75f}, false },
};
    const float raggio_cielo = 150.0f;
    glm::mat4 cielo(
        2.0f*raggio_cielo, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f*raggio_cielo, 0.0f, 0.0f,
        0.0f, 0.0f, 2.0f*raggio_cielo, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    sf::Clock clock;        
    float t_sim    = 0.0f;     // tempo della simulazione
    float t_prec   = 0.0f;     // per calcolare il delta
    float velocita = 1.0f;
    bool  pausa    = false;
    bool  mostra_orbite = true;

    const float dist_panoramica = sistema.back().dist_orbita * 2.0f;
    camera.set_distance(dist_panoramica);   
    int agganciato = 0;   // 0 = Sole

    window.setTitle("Solar System 3D  |  tasti 0-8  |  ora: " + sistema[agganciato].nome);
    std::cout << "\n=== COMANDI ===\n";
    for (size_t i = 0; i < sistema.size(); ++i)
        std::cout << "  " << i << "  ->  " << sistema[i].nome << "\n";
    std::cout << "\n"
                "  trascina tasto sinistro  ->  ruota la camera\n"
                "  LControl + mouse su/giu  ->  zoom\n"
                "  P                        ->  pausa / riprendi\n"
                "  freccia su / giu         ->  accelera / rallenta il tempo\n"
                "  O                        ->  mostra / nascondi le orbite\n"
                "  R                        ->  ripristina velocità \n"
            << std::endl;

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
                camera.set_aspect(static_cast<float>(resized->size.x),
                                  static_cast<float>(resized->size.y));
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
            else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                const int n = static_cast<int>(sistema.size());
                if (key->code >= sf::Keyboard::Key::Num0 && key->code <= sf::Keyboard::Key::Num8) {
                    int nuovo = static_cast<int>(key->code) - static_cast<int>(sf::Keyboard::Key::Num0);
                    if (nuovo < n && nuovo != agganciato) {
                        agganciato = nuovo;
                        const CorpoCeleste& c = sistema[agganciato];
                        if (agganciato == 0) camera.set_distance(dist_panoramica); // Sole, inquadra tutto il sistema
                        else camera.set_distance(c.raggio * 0.5f * 4.0f);  // pianeta, 4 raggi di distanza
                        window.setTitle("Solar System 3D  |  tasti 0-8  |  ora: " + c.nome);
                        std::cout << "camera agganciata a: " << c.nome << std::endl;
                    }
                }
                else if (key->code == sf::Keyboard::Key::P) {
                    pausa = !pausa;
                    std::cout << (pausa ? "pausa" : "ripresa") << std::endl;
                }
                else if (key->code == sf::Keyboard::Key::Up) {
                    velocita *= 1.5f;
                    if (velocita > 50.0f) velocita = 50.0f;
                    std::cout << "velocita: " << velocita << "x" << std::endl;
                }
                else if (key->code == sf::Keyboard::Key::Down) {
                    velocita /= 1.5f;
                    if (velocita < 0.05f) velocita = 0.05f;
                    std::cout << "velocita: " << velocita << "x" << std::endl;
                }
                else if (key->code == sf::Keyboard::Key::R) {
                    velocita = 1.0f;
                    std::cout << "velocita ripristinata: "
                    << velocita << "x" << std::endl;
                }
                else if (key->code == sf::Keyboard::Key::O) {
                    mostra_orbite = !mostra_orbite;
                }
            }
            
        }

        float t_reale = clock.getElapsedTime().asSeconds();
        float dt = t_reale - t_prec;
        t_prec = t_reale;

        if (!pausa) t_sim += dt * velocita;

        glm::mat4 m_target = sistema[agganciato].model(t_sim);
        camera.set_target(glm::vec3(m_target[3]));  
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform1i(tipoLoc, 3); //oggeto cielo 
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &cielo[0][0]);
        scene.draw();

        
        if(mostra_orbite){
            glUniform1i(tipoLoc, 2);
            glUniform3fv(objectColorLoc, 1, &coloreOrbita[0]);
            for (const CorpoCeleste& corpo : sistema) {
                if (corpo.dist_orbita <= 0.0f) continue; //il sole senza orbita

                float r = corpo.dist_orbita;
                glm::mat4 m(
                    r,    0.0f, 0.0f, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, r,    0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f
                );
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &m[0][0]);
                orbita.draw();
            }
        }
        

        for (const CorpoCeleste& corpo : sistema) {
            glm::mat4 model = corpo.model(t_sim);
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
            glUniform3fv(objectColorLoc, 1, &corpo.colore[0]);
            glUniform1i(tipoLoc, corpo.is_sun ? 1 : 0);
            scene.draw();
        }
        window.display();
    }

    return 0;
}
