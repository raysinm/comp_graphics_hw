#version 150

struct LightProperties
{
    vec4 position;
    vec4 dir;
    vec4 color;
    float La;
    float Ld;
    float Ls;
    int type;  // Ambient=0 / Point=1 / parallel=2
};

/* UBO */
layout(std140) uniform Lights
{
    LightProperties lights[10]; // Maximum number of lights
};


/* Input */
flat in vec3 flat_outputColor;
in vec3      outputColor;
in vec4      interpolated_normal;
in vec4      interpolated_position;
in vec2      st;

/* Material */
in vec3  interpolated_emissive;
in vec3  interpolated_diffuse;
in vec3  interpolated_specular;
flat in float interpolated_Ka;
flat in float interpolated_Kd;
flat in float interpolated_Ks;
flat in float interpolated_EmissiveFactor;
flat in int   interpolated_COS_ALPHA;

/* Textures*/
uniform sampler2D texMap;
uniform bool usingTexture;

/* Uniforms */
uniform vec3 wireframeColor;
uniform int algo_shading;
uniform int displayBBox;
uniform int displayVnormal;
uniform int displayFnormal;
uniform int numLights;

/* Output */
out vec4 FragColor;

/* Locals */
vec3  current_Color_emissive;
vec3  current_Color_diffuse;
vec3  current_Color_specular;
float current_Ka;
float current_Kd;
float current_Ks;
float current_EmissiveFactor;
int current_COS_ALPHA;

/* Functions */
vec3 calcIntensity(int i, vec3 P, vec3 N, int typeOfColor)
{
	vec3 I = vec3(lights[i].dir);                   // Light source direction to P (Assume Parallel light source)
	vec3 R = normalize((2 * dot(I, N) * N) - I);	// Direction of reflected light
	vec3 V = normalize(-P);							// Direction to COP (center of projection is the camera)
    vec3 lightSourceColor = vec3(lights[i].color);

    if(typeOfColor == 0)
    {
	    if (lights[i].type == 0) //Ambient
	    {
		    return vec3(lights[i].La * current_Ka * lightSourceColor);
	    }
        else
        {
            return vec3(0);
        }
    }
    else
    {
        if (lights[i].type == 0) //Ambient
        {
            return vec3(0);
        }

    	/* Recalculate the I and R because it was calculated for parallel light source */
		if (lights[i].type == 1) //Point Light
		{
			I = normalize(vec3(lights[i].position) - P);
			R = normalize((2 * dot(I, N) * N) - I);	// Direction of reflected light
		}

        if(typeOfColor == 1) //diffuse
        {
        	/* Calcualte diffuse */
		    float dotProduct_IN = max(0, dot(I, N));
		    return vec3( (lights[i].Ld * current_Kd * dotProduct_IN) * (lightSourceColor * current_Color_diffuse));
        }
        else if(typeOfColor == 2) //specular
        {
        	/* Calcualte specular */
		    float dotProduct_RV = pow( max(0, dot(R, V)), current_COS_ALPHA);
		    return vec3( (lights[i].Ls * current_Ks * dotProduct_RV) * (lightSourceColor * current_Color_specular) );
        }
    }

    return vec3(1,0,0);
}

vec3 getColor(vec4 point, vec4 normal)
{ 
    vec3 Ia_total   = vec3(0,0,0);
    vec3 Id_total   = vec3(0,0,0);
    vec3 Is_total   = vec3(0,0,0);

    vec3 P = vec3(point.x, point.y, point.z) / point.w;
    vec3 N = vec3(normal.x, normal.y, normal.z) / normal.w;

    for(int i = 0; i < numLights; i++)
    {
        Ia_total += calcIntensity(i, P, N, 0);
        Id_total += calcIntensity(i, P, N, 1);
        Is_total += calcIntensity(i, P, N, 2);
    }

    /* Add emissive light INDEPENDENT to any light source*/
    Ia_total += current_EmissiveFactor * current_Color_emissive;
    
    /* Add all 3 together and return the clamped color */
    vec3 result = Ia_total + Id_total + Is_total;

    return clamp(result, vec3(0), vec3(1));
}

/* Main */
void main()
{
    vec4 textureColor = texture2D(texMap, st);

    if(algo_shading == 0 || displayBBox == 1 || displayVnormal == 1 || displayFnormal == 1) //WireFrame
    {
	    FragColor = vec4(outputColor, 1);
    }
	else
    {
        if(algo_shading == 1)       // flat shading
        {
            FragColor = vec4(flat_outputColor, 1);
        }
        else if(algo_shading == 2)  // gououord shading
        {
            FragColor = vec4(outputColor, 1);
        }
        else if(algo_shading == 3)  // Phong shading
        {
            current_Color_emissive = interpolated_emissive;
            current_Color_diffuse  = interpolated_diffuse;
            current_Color_specular = interpolated_specular;
            current_Ka = interpolated_Ka;
            current_Kd = interpolated_Kd;
            current_Ks = interpolated_Ks;
            current_COS_ALPHA = interpolated_COS_ALPHA;
            current_EmissiveFactor = interpolated_EmissiveFactor;

            vec4 P = interpolated_position; //Vertex Position in CameraSpace
            vec4 N = interpolated_normal;   //Interpolated Normal in CameraSpace

        	FragColor = vec4(getColor(P, N), 1);
        }
    }
    

    if(usingTexture == true)
    {
        FragColor *= textureColor;
    }
} 

