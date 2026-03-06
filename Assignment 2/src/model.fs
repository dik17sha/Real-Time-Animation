#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 lightDir;
uniform vec3 viewPos;
uniform vec3 baseColor;

uniform sampler2D texture_diffuse1;

void main()
{
    vec3 n = normalize(Normal);
    vec3 L = normalize(-lightDir);

    float ambient = 0.25;
    float diffuse = max(dot(n, L), 0.0);

    vec3 tex = texture(texture_diffuse1, TexCoords).rgb;
    // If no texture is bound, this tends to be dark; blend toward baseColor for robustness.
    vec3 albedo = mix(baseColor, tex, 0.35);

    vec3 color = albedo * (ambient + 0.75 * diffuse);
    FragColor = vec4(color, 1.0);
}
