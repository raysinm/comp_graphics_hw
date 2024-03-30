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
in vec3 vPosition;
in vec3 fPosition;
in vec3 vn;
in vec3 fn;
in vec3 non_uniformColor_diffuse_FLAT;  //every 3 is duplicated to be the average of the face (to make a uniform same color for FLAT shading)
in vec3 non_uniformColor_diffuse;       //simple 1to1 mapping for every vertex - it's color
in vec2 texcoord;

/* Uniforms */
uniform int algo_shading;
uniform int displayBBox;
uniform int displayVnormal;
uniform int displayFnormal;
uniform int numLights;
uniform float vnFactor;
uniform float fnFactor;
uniform mat4 modelview;
uniform mat4 modelview_normals;
uniform mat4 projection;
uniform vec3 wireframeColor;

/* Material */
uniform vec3 uniformColor_emissive;
uniform vec3 uniformColor_diffuse;
uniform vec3 uniformColor_specular;
uniform float Ka;
uniform float Kd;
uniform float Ks;
uniform float EmissiveFactor;
uniform int COS_ALPHA;
uniform bool isUniformMaterial;

/* Output */
flat out vec3 flat_outputColor;
out vec3  outputColor;
out vec4  interpolated_normal;
out vec4  interpolated_position;
out vec3  interpolated_emissive;
out vec3  interpolated_diffuse;
out vec3  interpolated_specular;
flat out float interpolated_Ka;
flat out float interpolated_Kd;
flat out float interpolated_Ks;
flat out float interpolated_EmissiveFactor;
flat out int   interpolated_COS_ALPHA;
out vec2 st;

/* Locals */
vec4 vPos;
vec4 vPos_Cameraspace;
vec4 fPos;
vec4 resultPosition;
int vertexIndex;
vec3  current_Color_emissive;
vec3  current_Color_diffuse;
vec3  current_Color_specular;
float current_Ka;
float current_Kd;
float current_Ks;
float current_EmissiveFactor;
int current_COS_ALPHA;

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

void main()
{
    st = vec2(texcoord.x, 1 - texcoord.y);
    vPos = vec4(vPosition, 1);
    fPos = vec4(fPosition, 1);
    current_Color_emissive = uniformColor_emissive;
    current_Color_diffuse  = uniformColor_diffuse;
    current_Color_specular = uniformColor_specular;
    current_Ka = Ka;
    current_Kd = Kd;
    current_Ks = Ks;
    current_EmissiveFactor = EmissiveFactor;
    current_COS_ALPHA = COS_ALPHA;
    if(isUniformMaterial == false)
        current_Color_diffuse  = non_uniformColor_diffuse;

    vertexIndex = gl_VertexID;
    vPos_Cameraspace = modelview * vPos;
    resultPosition = projection * vPos_Cameraspace;


    if(displayBBox == 1)
    {
        outputColor = vec3(0, 1, 0);  // green
    }
    else if (displayFnormal == 1)
    {
        outputColor = vec3(0, 0, 1);  // blue
        if(vertexIndex % 2 == 1) 
        {
            vec4 normalDir = normalize(projection * normalize(modelview_normals * vec4(fn,1)));
            vec4 startPoint = resultPosition / resultPosition.w;
            resultPosition = startPoint + (normalDir * fnFactor);
            resultPosition.w = 1;
        }
    }
    else if (displayVnormal == 1)
    {
        outputColor = vec3(1, 0, 0);
        if(vertexIndex % 2 == 1) 
        {
            vec4 normalDir = normalize(projection * normalize(modelview_normals * vec4(vn,1)));
            vec4 startPoint = resultPosition / resultPosition.w;
            resultPosition = startPoint + (normalDir * vnFactor);
            resultPosition.w = 1;
        }
    }
    else // draw shading algos
    {
        if(algo_shading == 0) //wireframe
        {
            outputColor = wireframeColor;
        }
        else //  FLAT / GOUROUD / PHONG
        {
            if(algo_shading == 1) //flat shading
            {
                vec4 P = modelview * vec4(fPosition, 1);
                vec4 N = modelview_normals * vec4(fn, 1);
    	        
                if(isUniformMaterial == false)
                    current_Color_diffuse  = non_uniformColor_diffuse_FLAT;
        		
                flat_outputColor = getColor(P, N);
            }
            else if(algo_shading == 2) //Gouraud shading
            {
                vec4 P = vPos_Cameraspace;                //Vertex Position in CameraSpace
                vec4 N = modelview_normals * vec4(vn, 1); //Vertex Normal in CameraSpace

        		outputColor = getColor(P, N);
            }
            else if(algo_shading == 3) //Phong shading
            {
                interpolated_normal = modelview_normals * vec4(vn, 1);
                interpolated_position = vPos_Cameraspace;

                interpolated_emissive = current_Color_emissive;
                interpolated_diffuse  = current_Color_diffuse;
                interpolated_specular = current_Color_specular;
                interpolated_Ka = current_Ka;
                interpolated_Kd = current_Kd;
                interpolated_Ks = current_Ks;
                interpolated_EmissiveFactor = current_EmissiveFactor;
                interpolated_COS_ALPHA = current_COS_ALPHA;
            }
        }
    }

    gl_Position = resultPosition;
}
