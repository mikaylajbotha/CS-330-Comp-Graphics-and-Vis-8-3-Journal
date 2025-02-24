#version 330 core
out vec4 fragmentColor;

in vec3 fragmentPosition;
in vec3 fragmentVertexNormal;
in vec2 fragmentTextureCoordinate;

struct Material {
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
}; 

struct DirectionalLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    bool bActive;
};

struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    bool bActive;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
    bool bActive;
};

#define TOTAL_POINT_LIGHTS 5

uniform bool bUseTexture=false;
uniform bool bUseLighting=false;
uniform vec4 objectColor = vec4(1.0f);
uniform vec3 viewPosition;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[TOTAL_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;
uniform sampler2D objectTexture;
uniform vec2 UVscale = vec2(1.0f, 1.0f);

// the scaled texture coordinate to use in calculations
vec2 fragmentTextureCoordinateScaled = fragmentTextureCoordinate * UVscale;

// function prototypes
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{   
    if (!bUseLighting) {
        if (bUseTexture)
            fragmentColor = texture(objectTexture, fragmentTextureCoordinateScaled);
        else
            fragmentColor = objectColor;
        return;
    }

    if(bUseLighting == true)
    {
        vec3 phongResult = vec3(0.0f);
        // properties
        vec3 norm = normalize(fragmentVertexNormal);
        vec3 viewDir = normalize(viewPosition - fragmentPosition);
    
        // == =====================================================
        // Lighting is set up in 3 phases: directional, point lights, and optional spotlight
        // == =====================================================
        // phase 1: directional lighting
        if(directionalLight.bActive == true)
        {
            phongResult += CalcDirectionalLight(directionalLight, norm, viewDir);
        }
        // phase 2: point lights
        for(int i = 0; i < TOTAL_POINT_LIGHTS; i++)
        {
            if(pointLights[i].bActive == true)
            {
                phongResult += CalcPointLight(pointLights[i], norm, fragmentPosition, viewDir);   
            }
        } 
        // phase 3: spot light
        if(spotLight.bActive == true)
        {
            phongResult += CalcSpotLight(spotLight, norm, fragmentPosition, viewDir);    
        }
    
        if(bUseTexture == true)
        {
            fragmentColor = vec4(phongResult, (texture(objectTexture, fragmentTextureCoordinateScaled)).a);
        }
        else
        {
            fragmentColor = vec4(phongResult, objectColor.a);
        }
    }
    else
    {
        if(bUseTexture == true)
        {
            fragmentColor = texture(objectTexture, fragmentTextureCoordinateScaled);
        }
        else
        {
            fragmentColor = objectColor;
        }
    }
}


// calculates the color when using a directional light.
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
    if (!light.bActive) return vec3(0.0f); // Skip if inactive

    vec3 lightDirection = normalize(-light.direction);
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 reflectDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = light.ambient * (bUseTexture ? vec3(texture(objectTexture, fragmentTextureCoordinateScaled)) : vec3(objectColor));
    vec3 diffuse = light.diffuse * diff * material.diffuseColor * (bUseTexture ? vec3(texture(objectTexture, fragmentTextureCoordinateScaled)) : vec3(objectColor));
    vec3 specular = light.specular * spec * material.specularColor;

    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    if (!light.bActive) return vec3(0.0f); // Skip if inactive

    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = light.ambient * (bUseTexture ? vec3(texture(objectTexture, fragmentTextureCoordinateScaled)) : vec3(objectColor));
    vec3 diffuse = light.diffuse * diff * material.diffuseColor * (bUseTexture ? vec3(texture(objectTexture, fragmentTextureCoordinateScaled)) : vec3(objectColor));
    vec3 specular = light.specular * spec * material.specularColor;

    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    if (!light.bActive) return vec3(0.0f); // Skip if inactive

    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    if (theta < light.outerCutOff) return vec3(0.0f); // Skip if outside cone

    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = light.ambient * (bUseTexture ? vec3(texture(objectTexture, fragmentTextureCoordinateScaled)) : vec3(objectColor));
    vec3 diffuse = light.diffuse * diff * material.diffuseColor * (bUseTexture ? vec3(texture(objectTexture, fragmentTextureCoordinateScaled)) : vec3(objectColor));
    vec3 specular = light.specular * spec * material.specularColor;

    return intensity * (ambient + diffuse + specular);
}
