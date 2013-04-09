$input v_color0, v_sampleLeft, v_sampleRight

#include "common.sh"

SAMPLERCUBE(u_texColor, 0);

uniform float u_inverse_gamma;
//uniform float u_smoothness;

void main()
{   
   
    float left_dist = textureCube(u_texColor, v_sampleLeft.xyz).r;
    float right_dist = textureCube(u_texColor, v_sampleRight.xyz).r;   
    vec4 centerUV = 0.5 * (v_sampleRight +  v_sampleLeft);
    //float dist = textureCube(u_texColor, centerUV.xyz).r;
    float dist = 0.5 * (left_dist +  right_dist);        
    vec3 sub_color = smoothstep(0.5 -v_sampleLeft.w, 0.5 + v_sampleLeft.w,  vec3(left_dist, dist, right_dist));    
    //vec3 sub_color = smoothstep(0.5 -v_sampleLeft.w*0.75, 0.5 + v_sampleLeft.w*0.75,  vec3(left_dist, dist, right_dist));    
    //vec3 sub_color = smoothstep(minVal, maxVal,  vec3(left_dist, dist, right_dist));    
    //sub_colors.r = smoothstep(0.5 -delta, 0.5 + delta, left_distance);
    //sub_colors.g = smoothstep(0.5 -delta, 0.5 + delta, distance);
    //sub_colors.b = smoothstep(0.5 -delta, 0.5 + delta, right_distance);
    //float a = pow(sub_color.g, u_inverse_gamma);
    
    //mix with output color<
    //sub_colors + (vec3(1.0,1.0,1.0) - sub_color) * v_color0.rgb;


    float a = sub_color.g;
    
    //gl_FragColor.rgb = (a * v_color0.rgb);
    gl_FragColor.rgb = sub_color*v_color0.a;//(v_color0.rgb);// + (vec3(1.0,1.0,1.0) - sub_color) * (vec3(1.0,1.0,1.0) - v_color0.rgb);
    //(1.0-a) * sub_color;    
    float inverse = 1.0/1.5;
    gl_FragColor.rgb = pow(gl_FragColor.rgb, vec3(inverse,inverse,inverse));
    gl_FragColor.a = a*v_color0.a;
  //  gl_FragColor = pow(gl_FragColor, vec4(u_inverse_gamma,u_inverse_gamma,u_inverse_gamma,u_inverse_gamma));


    //gl_FragColor.rgb = v_color0.rgb * sub_color;// + ((vec3(1.0,1.0,1.0) - sub_color*v_color0.a) * v_color0.rgb *a);// a*(v_color0.rgb * (1.0 - a)) + (sub_color);
    //gl_FragColor.a = v_color0.a;//a;

    //sub_colors *= v_color0.a;

    //vec3 final_sub_colors = (sub_colors*v_color0.rgb) + (vec3(1.0,1.0,1.0) - sub_colors);
    

    //float a = sub_colors.g; 
    //gl_FragColor.r = (a * tR * v_color0.r) + ((1.0 - (a*tR)) * br);
    //AR,AG,AB are the intensities gotten from the subpixel rendering engine.
    //BR,BG,BB are the old background pixels.
    //DR,DG,DB are the new background pixels.
    //DR = A*AR*R + (1-(A*AR))*BR
    //DG = A*AG*G + (1-(A*AG))*BG
    //DB = A*AB*B + (1-(A*AB))*BB

    //float c = distance;  //0.5 * (left_distance + right_distance);
    //vec3 c1 = vec3(left_distance, c, right_distance);//(left_distance+ c+ right_distance)/3.0);
    //gl_FragColor.xyz = c1;// v_color0.rgb*c + c1*(1.0-c);
    //float a = pow(c, u_inverse_gamma);
    //gl_FragColor.rgb = (v_color0.rgb * c) + (c1 * (1.0-c));
    //a= a* v_color0.a;
    
    //if(a < 0.5)
    //{
      //gl_FragColor.a = a;// a;//1.0f;//a * v_color0.a ;
    //} else {
      //gl_FragColor.a = a;//1.0;// a;//1.0f;//a * v_color0.a ;
    //}


// uncorrected formula (same as GAMMA=1.0):
    // double output = input1 * alpha + input2 * (1.0 - alpha);
    // corrected formula:
    //double tmp = pow(input1, GAMMA) * alpha + pow(input2, GAMMA) * (1.0 - alpha);
    //double output = pow(tmp, 1.0/GAMMA);
    
    //gl_FragColor = vec4(left_distance, c, right_distance, c) + v_color0.w;

    //gl_FragColor.xyz = (vec3(1.0,1.0,1.0) - v_color0.rgb) * c + vec3(left_distance, c, right_distance) * (1.0 - c);
    //gl_FragColor.a = c*v_color0.w;
}
