#version 330



in vec4 vVaryingColor;
smooth in vec3 positionInEyeSpace3;
smooth in vec3 normalInEyeSpace;

out vec4 vFragColor;

struct pointLight{
	vec3 position; //eye space
	vec3 color;
	float attenuation0;
	float attenuation1;
	float attenuation2;
	float angle;
};

struct directionLight{
	vec3 direction;  //eye space
	vec3 color;
};

struct materialProperties{
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
	float specularExponent;
};


uniform pointLight light1;
uniform materialProperties material;



void main() {
	vFragColor.rgb = material.ambientColor;
	vFragColor.a = 1.0f;

	vec3 toLight = light1.position - positionInEyeSpace3;
	float r = length(toLight);
	float intensity = 1.0/(light1.attenuation0 + light1.attenuation1*r + light1.attenuation2*r*r);
	vec3 lightDirection = normalize(toLight);
	float diffuse = max(0.0, dot(normalInEyeSpace, lightDirection));

	vFragColor.rgb += intensity*diffuse*material.diffuseColor*light1.color;

	if(diffuse>0.0){
		vec3 halfVector = normalize(lightDirection - normalize(positionInEyeSpace3));
		float specular = max(0.0, dot(halfVector, normalInEyeSpace));
		float fspecular = pow(specular, material.specularExponent);
		vFragColor.rgb += intensity*fspecular*light1.color*material.specularColor;
	}
}