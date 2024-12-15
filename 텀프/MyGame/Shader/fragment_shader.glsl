#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D texture1;

const vec3 lightColor = vec3(1.f, 1.f, 1.f);	// 과제에선 uniform으로 해야할 것

const vec3 lightPos = vec3(10.f, 10.f, 0.f);

// 조명 정보
uniform vec3 cameraPos;

void main() {
    // Ambient lighting
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular lighting
    float specularStrength = 0.5;
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 lighting = ambient + diffuse + specular;
    vec4 textureColor = texture(texture1, TexCoords);
    FragColor = vec4(lighting * textureColor.rgb, textureColor.a);

}
