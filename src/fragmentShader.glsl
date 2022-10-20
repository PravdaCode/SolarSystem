#version 330 core	     // Minimal GL version support expected from the GPU

uniform vec3 camPos;
in vec3 fPosition;
out vec4 color;	  // Shader output: the color response attached to this fragment
in vec3 fColor;
out vec3 fNormal

void main() {
	vec3 n = normalize(fNormal);
	vec3 l = normalize(vec3(1.0, 1.0, 0.0)); // light direction vector (hard-coded just for now)
	// TODO: vec3 v = calculate view vector
	// TODO: vec3 r = calculate reflection vector
	// TODO: vec3 ambient = set an ambient color
	// TODO: vec3 diffuse = calculate the diffuse lighting
	// TODO: vec3 specular = calculate the specular lighting
	color = vec4(ambient + diffuse + specular, 1.0); // Building RGBA from RGB.
}
