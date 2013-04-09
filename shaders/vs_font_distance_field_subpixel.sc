$input a_position, a_color0, a_texcoord0
$output v_color0, v_sampleLeft, v_sampleRight

#include "common.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 0.0, 1.0) );	
	v_color0 = a_color0;
	v_sampleLeft = a_texcoord0;	
	v_sampleRight = a_texcoord0;

	float c = (0.667/512.0) * (16.0 * a_texcoord0.w);
	v_sampleLeft.x +=  c;
	v_sampleRight.x -=  c;
}
