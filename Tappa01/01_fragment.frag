#version 410 core

out vec4 fragment_color;
in vec3 interpolated_color; //normal (nx, ny, nz)

void main()
{
    vec3 lightDir = normalize(vec3(0.5, 0.7, 1.0)); // direzione luce fissa, arbitraria
    vec3 baseColor = vec3(0.8, 0.5, 0.2); // colore del pianeta (R, G, B)

    float diffuse = max(dot(normalize(interpolated_color), lightDir), 0.0); //luce diffusa Lambert
    vec3 ambient = baseColor * 0.18; // poca luce anche nella parte in ombra

    vec3 color = ambient + baseColor * diffuse;
    fragment_color = vec4(color, 1.0);
}
