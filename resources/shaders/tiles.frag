#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

struct Material {
    sampler2D diffuse;
    sampler2D specular;    
    float shininess;
}; 

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;  
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
};

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

uniform vec3 viewPos;
uniform DirLight dirLight[3];
uniform PointLight pointLights[2];
uniform SpotLight spotLight;
uniform Material material;

void main()
{
    if(texture(material.diffuse, TexCoord).a < 0.02f)   discard;

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
 
    vec3 result = vec3(0.0f);
    for(int i = 0; i < 3; i++){
        result += CalcDirLight(dirLight[i], norm, viewDir);
    }

    for(int i = 0; i < 2; i++){
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);   
    }

    //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);
    
    FragColor = vec4(result, texture(material.diffuse, TexCoord).a);
}


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir){
    vec3 ambient = (light.ambient * texture(material.diffuse, TexCoord).rgb);

    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = (light.diffuse * diff * texture(material.diffuse, TexCoord).rgb);

    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = (light.specular * spec * texture(material.specular, TexCoord).rgb);  
 
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));   
    
    vec3 ambient = (light.ambient * texture(material.diffuse, TexCoord).rgb) * attenuation;
    
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = (light.diffuse * diff * texture(material.diffuse, TexCoord).rgb) * attenuation;
    
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = (light.specular * spec * texture(material.specular, TexCoord).rgb) * attenuation;  
    
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
    vec3 lightDir = normalize(light.position - fragPos);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord)) * attenuation * intensity;
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord)) * attenuation * intensity;
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord)) * attenuation * intensity;
    return (ambient + diffuse + specular);
}