#version 400

in vec2 textureCoord; 
in vec3 norm;
in vec3 fragPos;

out vec4 fragColor;

uniform sampler2D texture1;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform vec3 waterColor;
uniform float fogDensity;

void main()
{
	// AMBIENT 
	float ka = 0.3;  
	vec3 ambient = ka * lightColor;

	//  DIFFUSE 
	
	vec3 normalVec = normalize(norm);
	vec3 lightDir = normalize(lightPos - fragPos);
	
	float kd = 1.0;  // diffuse coefficient of the material
	float diff = max(dot(normalVec, lightDir), 0.0);
	vec3 diffuse = kd * diff * lightColor;

	//  SPECULAR 
	
	float ks = 0.5;  
	
	vec3 viewDir = normalize(viewPos - fragPos);
	vec3 reflectDir = reflect(-lightDir, normalVec);
	
	float shininess = 32.0;
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	vec3 specular = ks * spec * lightColor;

	//  COMBINE ALL 
	vec3 lighting = ambient + diffuse + specular;
	vec4 objectColor = texture(texture1, textureCoord);
	vec3 result = lighting * objectColor.rgb;

	// UNDERWATER FOG 
	// Objects fade into blue water color based on distance
	float distance = length(viewPos - fragPos);
	float fogFactor = exp(-fogDensity * distance);  // Exponential fog
	fogFactor = clamp(fogFactor, 0.0, 1.0);
	
	// Mix between water color (far) and object color (close)
	result = mix(waterColor, result, fogFactor);

	fragColor = vec4(result, objectColor.a);
}