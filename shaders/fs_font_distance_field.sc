$input v_color0, v_texcoord0

#include "common.sh"

SAMPLERCUBE(u_texColor, 0);

uniform float u_inverse_gamma;
//uniform float u_smoothness;

void main()
{	
    float distance = textureCube(u_texColor, v_texcoord0.xyz).r;
    //const float smoothness = 16.0;
        //const float gamma = 2.2;   
    //float w = clamp( smoothness * 0.5f*(length(dFdx(v_texcoord0.xyz)) + length(dFdy(v_texcoord0.xyz))), 0.0, 0.5);
    //float a = smoothstep(0.5-w, 0.5+w, distance);   
    
    float a = smoothstep(0.5 -v_texcoord0.w, 0.5 + v_texcoord0.w , distance);    
    a = pow(a, u_inverse_gamma);
    gl_FragColor = vec4(v_color0.rgb, v_color0.a*a);
}