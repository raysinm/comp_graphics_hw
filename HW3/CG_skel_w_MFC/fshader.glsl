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
in vec3      vertPos;
flat in vec3      interpolatedTangent;
flat in vec3      interpolatedbBitangent;  
flat in vec3      nmN;
flat in mat3      TBN;
in vec3      vertPos_cameraspace;
in vec3      skyboxCoords;


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
uniform samplerCube skybox;
uniform sampler2D texMap;
uniform sampler2D normalMap;
uniform sampler2D texMarble;
uniform bool usingTexture;
uniform bool usingNormalMap;
uniform bool usingMarbleTex;
uniform int applyEnviornmentShading;


/* Uniforms */
uniform vec3 wireframeColor;
uniform int algo_shading;
uniform int displayBBox;
uniform int displayVnormal;
uniform int displayFnormal;
uniform int displaySkyBox;
uniform int numLights;
uniform int colorAnimateType;
uniform float smoothTime;
uniform float minX;
uniform float maxX;
uniform vec3 cameraPos;
uniform vec2 resolution;
uniform vec3 mcolor1, mcolor2;

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
float map(float val, float minimum, float maximum)
{
    return (val - minimum) / (maximum - minimum);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

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

mat3 calcTBN(vec3 N)
{
    return mat3(interpolatedTangent, interpolatedbBitangent, N);
}

mat3 calcTBN(vec4 N)
{
    return mat3(interpolatedTangent, interpolatedbBitangent, vec3(N)); // MAYBE WRONG
}

vec3 calcNormalTangent(vec3 N)
{
    mat3 _TBN = calcTBN(N);
        // Normal map calcs
    vec3 normal_from_map = texture(normalMap, st).rgb; // Check if useNormalMap is true?
    normal_from_map.y = 1 - normal_from_map.y;
    normal_from_map = normal_from_map*2.0 -1.0;  // Normalize to [-1,1]
   return normalize(_TBN * normal_from_map); // Transform from tangent space to modelview space

}

vec4 calcNormalTangent(vec4 N)
{
    return vec4(calcNormalTangent(vec3(N)), 1.0);
}

vec3 marbleColor(vec2 uv)
{
    float a = (sin(uv.x) + 1) / 2;
	float b = 1 - (sin(uv.x) + 1) / 2;
	return a * mcolor1 + b * mcolor2;
}

/* Main */
void main()
{
    vec4 textureColor;
    vec3 normal_from_map;

    bool drawingTriangles = false;

    if(applyEnviornmentShading == 1 && displaySkyBox == 1)
    {
        FragColor = texture(skybox, skyboxCoords);
        return;
    }


    if(algo_shading == 0 || displayBBox == 1 || displayVnormal == 1 || displayFnormal == 1) //WireFrame
    {
	    FragColor = vec4(outputColor, 1);
    }
	else
    {
        drawingTriangles = true;
        textureColor = texture2D(texMap, st);   // Check if useTexture is true?



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
            if(usingNormalMap)
            {
                mat3 _TBN = mat3(interpolatedTangent, interpolatedbBitangent, nmN);
                vec3 normal_from_map = texture2D(normalMap, st).rgb; // Check if useNormalMap is true?
                //normal_from_map.y = 1 - normal_from_map.y;
                normal_from_map = normal_from_map*2.0 -1.0;  // Normalize to [-1,1]
                vec3 _N = -normalize(_TBN * normal_from_map); // Transform from tangent space to modelview space
                N = vec4(_N, 1.0);
            }

            if(usingMarbleTex)
            {
                vec2 uv = gl_FragCoord.xy / resolution;
                vec3 marble_color = marbleColor(uv); 
                current_Color_diffuse = marble_color;
            }

        	FragColor = vec4(getColor(P, N), 1);
        }
    
    

        if(drawingTriangles == true)
        {
            if(usingTexture == true)
            {
                FragColor *= textureColor;
            }
            else if(applyEnviornmentShading == 1)
            {
                vec3 I = normalize(vertPos_cameraspace);
                vec3 N = normalize(interpolated_normal.xyz);
               
                vec3 R = reflect(I, N);

                FragColor = vec4(texture(skybox, R).rgb, 1.0);
            }
        }

        if(colorAnimateType == 1)
        {
            vec3 hsvColor = vec3(smoothTime, 1.0, 1.0);
            FragColor += vec4(hsv2rgb(hsvColor)/2, 0);
        }
        else if(colorAnimateType == 2)
        {
            float PI = 3.14159265359;
            float arg = map(vertPos.x, minX, maxX);
            float t = 0.5 + sin(PI * (arg - 0.5f))/2;
            vec3 hsvColor = vec3(t + smoothTime, 1.0, 1.0);
            vec3 animateColor = hsv2rgb(hsvColor) / 2;
            FragColor += vec4(animateColor, 0);
        }
    }
}

