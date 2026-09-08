#ifndef MESH_HH
#define MESH_HH

#include <vector>
#include <cmath>

#include <glm/glm.hpp>

class Mesh{
    private:
    std::vector<glm::vec3> vertices; //(x, y, z)
    std::vector<glm::uvec3> triangles; //tripla di indici dei vertici 
    std::vector<glm::vec3> normals; //normali -> frecce che indicano la dir della superficie 
    std::vector<glm::vec2> uv; //coordinate texture (u,v)
    public:
    static Mesh crea_sfera(unsigned int sectorCount = 32, unsigned int stackCount = 16, float radius = 1.0f  ){
        Mesh mesh;
        mesh.build_sphere(sectorCount, stackCount, radius );
        mesh.compute_normals();
        return mesh;
    }
    //trasforma i dati in forma per riempire i buffer OpenGL(VBO/EBO)
    void pack4gpu(std::vector<float>& points, std::vector<unsigned int>& indices){
        points={};
        for(unsigned int i=0; i<vertices.size(); i++){
            //posizione 
            points.push_back(vertices[i].x);
            points.push_back(vertices[i].y);
            points.push_back(vertices[i].z);
            //normale
            points.push_back(normals[i].x);
            points.push_back(normals[i].y);
            points.push_back(normals[i].z);
            //coordinate texture
            points.push_back(uv[i].x);
            points.push_back(uv[i].y);
        }
        indices={};
        for (auto t : triangles)
            for (unsigned i = 0; i < 3; i++)
                indices.push_back (t[i]);
    }

    private:
    void build_sphere(unsigned int sectorCount, unsigned int stackCount, float radius){
        vertices.clear();
        triangles.clear();
        uv.clear();

        float sectorStep = 2 * M_PI/sectorCount;
        float stackStep = M_PI/stackCount;
        float sectorAngle, stackAngle;
        for(unsigned int i=0; i<=stackCount; i++){
            stackAngle = M_PI / 2 - i * stackStep; // da +90° a -90°, phi nella formula
            float xz = radius * cosf(stackAngle); // r * cos(u) -> raggio anello
            float y = radius * sinf(stackAngle); // r * sin(u) -> altezza anello
            for(unsigned int j=0; j<=sectorCount; j++){
                sectorAngle=j*sectorStep;
                float x = xz * cosf (sectorAngle); // r * cos(u) * cos(v)
                float z = -xz * sinf (sectorAngle); // r * cos(u) * sin(v)

                vertices.emplace_back (x, y, z);
                uv.emplace_back((float)j/sectorCount, (float)i/stackCount);

            }
        }
        //genera triangoli tra anelli consecutivi
        //ogni settore in una pila richiede 2 triangoli
        for(unsigned int i=0; i<stackCount;i++){
            unsigned int k1=i*(sectorCount + 1); //vertice anello corrente
            unsigned int k2 = k1 + (sectorCount + 1); //vertice anello successivo

            for(unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2){
                if (i != 0) 
                    triangles.push_back (glm::uvec3 (k1, k2, k1 + 1));
                if(i != (stackCount - 1)) 
                    triangles.push_back (glm::uvec3 (k1 + 1, k2, k2 + 1));
            }
        }

    }

    private:
    void compute_normals(){
        normals.assign (vertices.size (), glm::vec3 (0.0f));
        //ciclo su tutti i triangoli della mesh 
        for(const auto& t: triangles){
            const glm::vec3& v0 = vertices[t[0]];
            const glm::vec3& v1 = vertices[t[1]];
            const glm::vec3& v2 = vertices[t[2]];
            glm::vec3 Nt = glm::cross (v1 - v0, v2 - v0);
            normals[t[0]] += Nt;
            normals[t[1]] += Nt;
            normals[t[2]] += Nt;
        }
        for (auto& n : normals) {
            n = glm::normalize (n);
        }
    }

};

#endif