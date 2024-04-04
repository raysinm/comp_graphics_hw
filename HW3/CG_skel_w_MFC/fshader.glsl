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
in mat3      TBN;
in vec3      outputColor;
in vec4      interpolated_normal;
in vec4      interpolated_position;
in vec2      st;
in vec3      vertPos;
in vec3      vertPos_worldspace;
in vec3      normal_worldspace;
in vec3      skyboxCoords;


/* Material */
flat in float interpolated_Ka;
flat in float interpolated_Kd;
flat in float interpolated_Ks;
flat in float interpolated_EmissiveFactor;
flat in int   interpolated_COS_ALPHA;
in vec3       interpolated_emissive;
in vec3       interpolated_diffuse;
in vec3       interpolated_specular;

/* Textures*/
uniform sampler2D texMap;       //GL_TEXTURE0
uniform sampler2D normalMap;    //GL_TEXTURE1
uniform sampler2D texMarble;    //GL_TEXTURE2
uniform samplerCube skybox;     //GL_TEXTURE3
uniform bool usingTexture;
uniform bool usingNormalMap;
uniform bool usingMarbleTex;
uniform int applyEnviornmentShading;


/* Uniforms */
uniform vec3  wireframeColor;
uniform vec3  cameraPos;
uniform float smoothTime;
uniform float minX, minY, minZ;
uniform float maxX, maxY, maxZ;
uniform vec3  mcolor1, mcolor2;
uniform int   algo_shading;
uniform int   displayBBox;
uniform int   displayVnormal;
uniform int   displayFnormal;
uniform int   displaySkyBox;
uniform int   displayCameraIcon;
uniform int   numLights;
uniform int   colorAnimateType;

uniform float veinFreq;
uniform int veinThickness;
uniform float colMixFactor;
uniform float noiseFreq;
uniform int noiseOctaves;
uniform float noiseAmplitude;

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
float PI = 3.14159265359;
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

vec4 calcNormalTangent()
{
    vec3 normal_from_map = texture2D(normalMap, st).rgb * 2.0 - 1.0; // Normalize to [-1,1]
    return vec4(normalize(TBN * normal_from_map), 1);
}


vec3 marble_color(float x)
{
    vec3 col;
    x = (x + 1) / 2;          // Normalize to [0,1]
    
    for (int i=0; i < veinThickness; i++)
    {
        x = sqrt(x);              // vein effect
    }    
    //col = vec3(.2 + .75*x);           //option 1
    //col.b*=0.95;             
    //col = vec3(x);                    //option 2
    col = mix(mcolor2, mcolor1, colMixFactor*x);     //option 3

    return col;
}


float turbulence(vec3 p, int octaves)
{
    float val = 0.0;
    float noise_freq = noiseFreq;
    for (int i=0; i<octaves; i++)
    {
        val += texture2D(texMarble, (vec2(p.x,p.y) * noise_freq)).r * 2.0 - 1.0;
        noise_freq *= 2.07;
    }
    return val;
}


/* Main */
void main()
{
    vec4 textureColor;
    vec3 normal_from_map;


    if(applyEnviornmentShading == 1 && displaySkyBox == 1)
    {
        FragColor = texture(skybox, skyboxCoords);
        return;
    }


    if(algo_shading == 0 || displayBBox == 1 || displayVnormal == 1 || displayFnormal == 1 || displayCameraIcon == 1) //WireFrame
    {
	    FragColor = vec4(outputColor, 1);
    }
	else
    {

        if(usingTexture == true)
          textureColor = texture2D(texMap, st);

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
                //vec3 normal_from_map = texture2D(normalMap, st).rgb * 2.0 - 1.0; // Normalize to [-1,1]
                //N = vec4(normalize(TBN * normal_from_map), 1);
                N = calcNormalTangent();
            }


        	FragColor = vec4(getColor(P, N), 1);
        }
    
        
        if(applyEnviornmentShading == 1 && !usingTexture && !usingMarbleTex)
        {
            vec3 I = normalize(vertPos_worldspace - cameraPos);
            vec3 N = normalize(normal_worldspace.xyz);
               
            vec3 R = reflect(I, N);

            FragColor = vec4(texture(skybox, R).rgb, 1.0);
        }


        if(usingMarbleTex == true)
        {
            float xPos = map(vertPos.x, minX, maxX);
            float yPos = map(vertPos.y, minY, maxY);
            float zPos = map(vertPos.z, minZ, maxZ);
            vec3 normPos = vec3(xPos, yPos, zPos);
                
            float t = mix(xPos, yPos, 0.45);
            t *= veinFreq;
            t += noiseAmplitude * turbulence(normPos, noiseOctaves);

            vec3 marbleColor = marble_color(sin(t));

            FragColor = mix(FragColor, vec4(marbleColor, 1), 0.5);
        }

        if(usingTexture == true)
        {
            FragColor *= textureColor;
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

