#version 410 core

out vec4 fragment_color;
in vec3 interpolated_normal; //normal (nx, ny, nz)
in vec3 fragment_position;
uniform vec3 objectColor;
uniform vec3 sunPosition;
uniform int tipo; //0 pianeta, 1 sole, 2 orbita
in vec3 local_position;

void main()
{
    if(tipo==2){
        fragment_color = vec4(objectColor, 1.0);
        return;
    }
    if(tipo==1){
        float brightness = 1.0 + 0.2 * sin(local_position.x * 15.0) * cos(local_position.z * 15.0);
        fragment_color = vec4(objectColor * brightness, 1.0);
        return;
    }
    //pianeti
    vec3 lightDir = normalize(sunPosition - fragment_position); // Direzione dal punto della superficie verso il Sole
    float ambient=0.09;
    float diffuse = max(dot(normalize(interpolated_normal), lightDir), 0.0); //luce diffusa Lambert
    vec3 color = objectColor * (ambient+diffuse);

    fragment_color = vec4(color, 1.0);
}
