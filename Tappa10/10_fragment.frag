#version 410 core

out vec4 fragment_color;
in vec3 interpolated_normal; //normal (nx, ny, nz)
in vec3 fragment_position;
in vec2 uv;

uniform vec3 objectColor;
uniform vec3 sunPosition;
uniform int tipo; //0 pianeta, 1 sole, 2 orbita
in vec3 local_position;
uniform sampler2D planetTexture;

uniform vec3 camera_pos;
uniform float ks;
uniform float esp_p;

float pc(vec3 p){ //restituisce numero tra 0 e 1
    float n = p.x * 127.0 + p.y * 311.0 + p.z * 75.0;
    return fract(sin(n) * 10000.0);
}

void main()
{
    if(tipo==2){
        fragment_color = vec4(objectColor, 1.0);
        return;
    }
    else if(tipo==1){
        fragment_color = texture(planetTexture, uv);
        return;
    }
    // cielo stellato
    else if (tipo == 3) {
        vec3 d = normalize(interpolated_normal);
        vec3 c = d * 150.0; //ingrandimo coordinate
        vec3 cella  = floor(c); //elimina parte decimale
        vec3 dentro = fract(c) - 0.5; //otteniamo intervallo da -0.5 a +0.5 per sapere dove sono nella cella 
        float h = pc(cella); //numero pseudo casuale 
        float accesa  = step(0.99, h); //se h < 0.99 -> 0, se h >= 0.99 -> 1 stella
        float sfumata = smoothstep(0.35, 0.0, length(dentro));// luminosa al centro, spenta ai bordi
        float lum = accesa * sfumata * (0.4 + 0.6 * h);         // luminosità diversa per stella
        float t = pc(cella + 17.0);
        vec3 tinta = mix(vec3(0.7,0.8,1.0), vec3(1.0,0.85,0.7), t); 
        fragment_color = vec4(tinta * lum, 1.0);
        return;
    }
    else if (tipo == 4) {
        fragment_color = texture(planetTexture, uv);
        return;
    }
    //pianeti
    vec3 lightDir = normalize(sunPosition - fragment_position);
    vec3 n = normalize(interpolated_normal);
    vec3 v  = normalize(camera_pos - fragment_position);
    vec3 h = normalize(v + lightDir); //h=v+l/|v+l| bisettrice di v e l

    float ambient = 0.09;
    float diffuse = max(dot(normalize(interpolated_normal), lightDir), 0.0);
    float speculare=0.0;
    if(diffuse>0) speculare = ks * pow(max(dot(n, h), 0.0), esp_p);

    vec3 base = texture(planetTexture, uv).rgb;
    vec3 color = base * (ambient + diffuse) + vec3(speculare);
    fragment_color = vec4(color, 1.0);
}
