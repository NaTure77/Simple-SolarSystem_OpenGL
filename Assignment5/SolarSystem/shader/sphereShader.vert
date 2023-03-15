//#version 120
#version 150
in vec4 vPosition;
in vec3 vTexCoord;
out vec2 texCoord;

uniform mat4 projMatrix;
uniform mat4 modelMatrix;
uniform mat4 distanceMat_view;
uniform vec4 rquat;



out vec3 fN;
out vec3 fE;
out vec3 fL;

vec4 multq (vec4 a, vec4 b){
	return vec4(a.x * b.x - dot(a.yzw, b.yzw), a.x * b.yzw + b.x * a.yzw + cross(b.yzw , a.yzw));
}

vec4 invq(vec4 a){
	return vec4(a.x, -a.yzw) / dot(a,a);
}

void main()
{
	vec4 p = vec4(0.0, modelMatrix * vPosition);
	p = multq(rquat, multq(p, invq(rquat)));
	
	vec4 basePos = modelMatrix * vec4(0,0,0,1);
	basePos = vec4(0, basePos.xyz);
	basePos = multq(rquat, multq(basePos, invq(rquat)));
	
	
	vec4 v = vec4(p.yzw, 1);
	
	fN = normalize(v.xyz - basePos.yzw);
	fE = -(distanceMat_view * v).xyz;
	fL = -v.xyz;
	gl_Position = projMatrix * distanceMat_view * v;
	
	texCoord = vTexCoord.xy;
}