$input v_color0, v_texcoord0

#include "common.sh"

SAMPLERCUBE(u_texColor, 0);

uniform float u_inverse_gamma;

void main()
{	
	float a = textureCube(u_texColor, v_texcoord0.xyz).r;
	a = pow(a, u_inverse_gamma);
	gl_FragColor = vec4(v_color0.rgb, v_color0.a * a);    
}
