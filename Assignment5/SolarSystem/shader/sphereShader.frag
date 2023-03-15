//#version 120
#version 150
in vec2 texCoord;
uniform sampler2D texture;

in vec3 fN;
in vec3 fE;
in vec3 fL;

//uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
//uniform mat4 ModelView;
//uniform float shininess;
uniform bool shading;
uniform bool texturing;
void main(){

	vec4 result = vec4(1,1,1,1);
	if(shading)
	{
		vec3 N = normalize(fN);
		vec3 E = normalize(fE);
		vec3 L = normalize(fL);
		vec3 H = normalize(L + E);
		//vec4 ambient = AmbientProduct;
		vec4 ambient = vec4(0.2, 0.2, 0.2, 1.0);
		
		float kd = max(dot(L, N), 0.0);
		//vec4 diffuse = kd * DiffuseProduct;
		vec4 diffuse = kd * vec4(1.0, 0.8, 0.6, 1.0);
		
		float ks = pow(max(dot(N, H), 0.0), 100);
		vec4 specular = ks * vec4(1.0,1.0,1.0,1.0);
		if(dot(L, N) < 0.0)
		{
			specular = vec4(0.0, 0.0, 0.0, 1.0);
		}
		
		vec4 c = ambient + diffuse + specular;
		c.a = 1.0;
		
		result *= c;
		//gl_FragColor = c * texture2D(texture, texCoord);
	}
	if(texturing)
		result *= texture2D(texture, texCoord);

	gl_FragColor = result;
}