#version 330 core
out vec4 FragColor;

in vec3 FragPos;  
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 lightPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform sampler2D texture1;

void main()
{
	vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 result = diffuse * objectColor;
	FragColor = texture(texture1, TexCoord) * vec4(result, 1.0);
}