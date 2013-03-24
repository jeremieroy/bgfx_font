$input v_color0, v_texcoord0

#include "common.sh"

SAMPLERCUBE(u_texColor, 0);


void main()
{	
    float distance = textureCube(u_texColor, v_texcoord0.xyz).r;
    const float smoothness = 16.0;
    const float gamma = 2.2;    
    float w = clamp( smoothness * 0.5f*(length(dFdx(v_texcoord0.xyz)) + length(dFdy(v_texcoord0.xyz))), 0.0, 0.5);
    float a = smoothstep(0.5-w, 0.5+w, distance);   

    //float w = clamp( smoothness * fwidth(distance) , 0.0, 0.5);

    
    a = pow(a, 1.0/gamma);
    gl_FragColor = vec4(v_color0.rgb, v_color0.a*a);
    
  	
  	//float smoothing = fwidth(distance);
  	// Perform anisotropic analytic antialiasing (fwidth() is slightly wrong)
    //float smoothing = length(vec2(dFdx(distance), dFdy(distance)));
    //const float smoothing = 1.0/16.0;

    //float alpha = 1.0;
    // perform simple thresholding
    //if( distance < 0.5 - smoothing ) alpha= 0.0;

    //const float smoothing = 1.0/4.0;
    //alpha *= smoothstep(0.5-smoothing , 0.5 +smoothing , distance);
    
    // do some anti-aliasing
    //alpha *= smoothstep(0.45, 0.55, distance);

    //if(distance < 0.5) alpha = 0.0;
    //else alpha = 1.0;
    //const float smooth_min = 0.45;
    //const float smooth_max = 0.55;    
    //float alpha = smoothstep(smooth_min, smooth_max, distance);
    //gl_FragColor = vec4(v_color0.rgb, alpha);
}