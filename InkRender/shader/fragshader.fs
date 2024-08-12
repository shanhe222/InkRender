#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in float vDotN;
in float Curvature;


uniform sampler2D texture_diffuse1;  // Regular texture
uniform sampler2D texture_ink1;      // Ink texture
uniform sampler2D texture_Brushstrokes1; 

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform int mixValue;  // 传入的参数
uniform float edgeRange;
uniform float edgeThred;
uniform float edgePow;
uniform float lightStrength;
uniform int kEnabled; // 接收从C++传递的布尔变量
// 定义内描边参数为常量

uniform sampler2D diffinkTexture;
uniform float diffusionRate;
uniform vec2 texelSize;



float depthFactor(float depth) {
    float near = 0.1; // 近裁剪面
    float far = 1.0; // 远裁剪面
    float z = depth * 2.0 - 1.0; // 将深度从 [0, 1] 范围映射到 [-1, 1]
    return (2.0 * near * far) / (far + near - z * (far - near));
}

// 计算初始颜色值（根据需要调整映射函数）
float computeInitialColorValue(float k) {
    if (k <= 0.1) {
        return 0.0; // 空白区域
    } else {
        return k * 255.0; // 示例：将曲率值映射到颜色值
    }
}

// 计算最终颜色值，基于公式(12)
float computeColorValue(float C_j) {
    return 1.0 - pow(1.0 + 0.01 * C_j * C_j, -0.5);
}

void main() {

    
    //Depth
    float depth = gl_FragCoord.z;
    //Brush Tex
    vec4 brushcolor = texture(texture_Brushstrokes1, TexCoords);
    
    //——————Caculate Lighting——————
    // Ambient component
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse component
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular component
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * lightColor;

    float shadowThreshold = 0.7; // Shadow threshold
    vec3 lighting = (diffuse) * lightStrength;
    // Calculate sharper shadows
    vec3 lighting2 = vec3(step(shadowThreshold, lighting.r),
                        step(shadowThreshold, lighting.g),
                        step(shadowThreshold, lighting.b)) * lighting;

    bool isInShadow = dot(lighting, vec3(0.299, 0.587, 0.114)) < shadowThreshold; // Calculate whether in shadow using luminance

    // Use brush texture in shadow, otherwise use regular lighting
    vec3 lighting3;
    if (isInShadow) {
        lighting3 = mix(lighting2, vec3(brushcolor.rgb), 0.5); // Use the color of the brush texture
    } else {
        lighting3 = lighting2;
    }
    vec3 lighting4 = clamp(lighting3, 0.0, 1.0) * 0.5; // Correction

    

    vec4 color = texture(texture_diffuse1, TexCoords);

    //——————Texture MIx——————
    

   vec2 worldUV = FragPos.xy * 1.0; // Adjust this factor as needed
    vec4 inkColor = texture(texture_ink1, TexCoords);

    // Use a dynamic blend factor to mix two textures
    vec4 finalColor;
    // Calculate the grayscale value of the main texture
    float texGrey = (color.r + color.g + color.b) * 0.33;
    texGrey = pow(texGrey, 0.3);
    texGrey *= 1.0 - cos(texGrey * 3.14);

    // Calculate the grayscale value of the brush texture
    float brushGrey = (inkColor.r + inkColor.g + inkColor.b) * 0.33;

    // Calculate the final color
    finalColor = vec4(texGrey * brushGrey, texGrey * brushGrey, texGrey * brushGrey, 1.0);

    // ——————Calculate Edge and blend with texture————



    // Adjust vDotN based on _Range and apply power for initial edge enhancement
    float edge = pow(vDotN, 1.0) / edgeRange;
    float e = vDotN ;
    
    // Apply threshold
    edge = edge > edgeThred ? 1.0 : edge;

    // Apply power for contrast adjustment
    edge = pow(edge, edgePow);

    // Adjust edge width based on depth
    float depthAdj = depthFactor(depth);
    edge *= depthAdj; // Adjust edge intensity using depth


    //----- Blend Black Edges and Color -----

    // If edge is close to 1 (i.e., at the edge), use black; otherwise, use the texture color
    vec3 color2 = mix(vec3(0.0), finalColor.rgb, edge);

    // Use the mixValue parameter to control whether to use brush texture to adjust the edge
    if( mixValue == 1)
    {
        if (color2.r < 0.05 && color2.g < 0.05 && color2.b < 0.05) {//focus on black edge
            color2 = finalColor.rgb - vec3(brushcolor.a);
        }
    }
    
    vec3 color3 = mix(color2, color2 * 0.5 + lighting2, lightStrength);

   // ——————Curvature's effect on ink color——————
    
    // Calculate the changes in normal and position
    // Calculate the variation in normal and position
    vec3 dNdx = dFdx(Normal);
    vec3 dNdy = dFdy(Normal);
    vec3 dPdx = dFdx(FragPos) * 100;
    vec3 dPdy = dFdy(FragPos) * 100;

    // Calculate curvature
    float curvature = length(dNdx) / length(dPdx) + length(dNdy) / length(dPdy);

    // Calculate surface volume
    float V_surface = curvature > 0.1 ? 1.0 : 0.0;

    // Calculate k value

    float k;
    if (curvature <= 0.05) k = 0.0;
    else if (curvature <= 0.06) k = 0.5;
    else if (curvature <= 0.07) k = 0.8;
    //else if (curvature <= 0.2) k = 0.7;
    else k = 0.8;

    // Calculate surface mass
    float rho_ink = 1.0; // Assume ink density is 1.0
    float omega = 1.0; // Assume constant omega is 1.0
    float m_surface = rho_ink * V_surface * pow(1.0 + omega / k, -1.0);

    // Render using the surface mass m_surface
    vec4 currentInk = texture(texture_diffuse1, gl_FragCoord.xy * texelSize);
    vec4 neighborInk = texture(texture_diffuse1, gl_FragCoord.xy * texelSize + texelSize * vec2(1.0, 0.0)) +
                    texture(texture_diffuse1, gl_FragCoord.xy * texelSize + texelSize * vec2(-1.0, 0.0)) +
                    texture(texture_diffuse1, gl_FragCoord.xy * texelSize + texelSize * vec2(0.0, 1.0)) +
                    texture(texture_diffuse1, gl_FragCoord.xy * texelSize + texelSize * vec2(0.0, -1.0));

    vec4 averageNeighborInk = neighborInk / 4.0;
    vec4 diffusedInk = mix(currentInk, averageNeighborInk, diffusionRate);

    // Convert ink properties to screen pixel color values
    float C_i = m_surface; // Use m_surface as the ink concentration

    float C = 1.0 - pow(1.0 + 0.01 * C_i * C_i, -0.5);
    C = C * 1000 + 0.1;
    
    
    float m = (1- curvature)*(1- curvature);
   
    vec4 whiteColor = vec4(1.0, 1.0, 1.0, 1.0);

   // Use alpha blending, where the alpha value of the translucent image determines the final color
    vec3 color4 = mix(whiteColor.rgb, color3.rgb, k);
    color4 = color3.rgb * C + color4 * 0.5;

    // Set the fragment color
    FragColor = vec4(color3.rgb, 1.0); // Assume using grayscale color

    if(kEnabled == 1)
        FragColor = vec4(color4, 1.0); 
    
}

