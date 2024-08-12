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

    
    //深度
    float depth = gl_FragCoord.z;
    //笔刷纹理
    vec4 brushcolor = texture(texture_Brushstrokes1, TexCoords);
    

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

    float shadowThreshold = 0.7; // 阴影阈值
    vec3 lighting = ( diffuse ) * lightStrength;
    // 计算更锋利的阴影
    vec3 lighting2 = vec3(step(shadowThreshold, lighting.r),
                                step(shadowThreshold, lighting.g),
                                step(shadowThreshold, lighting.b)) * lighting;

    bool isInShadow = dot(lighting, vec3(0.299, 0.587, 0.114)) < shadowThreshold; // 使用亮度计算是否在阴影中

    // 在阴影中使用笔刷纹理，否则使用常规光照
    vec3 lighting3;
    if (isInShadow) {
        lighting3 = mix(lighting2, vec3(brushcolor.rgb), 0.5); // 使用笔刷纹理的颜色
    } else {
        lighting3 = lighting2;
    }
    vec3 lighting4 = clamp(lighting3, 0.0, 1.0)*0.5; // 校正
    

    vec4 color = texture(texture_diffuse1, TexCoords);
    //color = vec4(color.r,color.r,color.r,1.0);
    

    vec2 worldUV = FragPos.xy * 1.0; // 适当调整这个因子
    vec4 inkColor = texture(texture_ink1, TexCoords);
   
    
    // 使用动态混合因子混合两个纹理
    vec4 finalColor;
    // 计算主纹理的灰度值
    float texGrey = (color.r + color.g + color.b) * 0.33;
    texGrey = pow(texGrey, 0.3);
    texGrey *= 1.0 - cos(texGrey * 3.14);

    // 计算画笔纹理的灰度值
    float brushGrey = (inkColor.r + inkColor.g + inkColor.b) * 0.33;

    // 计算最终的颜色
    finalColor = vec4(texGrey * brushGrey, texGrey * brushGrey, texGrey * brushGrey, 1.0);

    
    
    // 计算法线和位置的变化
     // 计算法线和位置的变化量
    vec3 dNdx = dFdx(Normal);
    vec3 dNdy = dFdy(Normal);
    vec3 dPdx = dFdx(FragPos)*100;
    vec3 dPdy = dFdy(FragPos)*100;

    // 计算曲率
    float curvature = length(dNdx) / length(dPdx) + length(dNdy) / length(dPdy);

    // 线性插值，将基础颜色与白色进行混合
    vec3 finalColor2 = mix(vec3(1.0, 1.0, 1.0), finalColor.rgb, curvature);
    //finalColor = vec4(finalColor2,1.0);



    // Adjust vDotN based on _Range and apply power for initial edge enhancement
    float edge = pow(vDotN, 1.0) / edgeRange;
    float e = vDotN ;
    
    // Apply threshold
    edge = edge > edgeThred ? 1.0 : edge;

    // Apply power for contrast adjustment
    edge = pow(edge, edgePow);

    // 根据深度调整边缘宽度
    float depthAdj = depthFactor(depth);
    edge *= depthAdj; // 使用深度调整边缘强度

    //----- Blend Black Edges and Color -----

    // If edge is close to 1 (i.e., at the edge), use black; otherwise, use the texture color
    vec3 color2 = mix(vec3(0.0), finalColor.rgb, edge);

    // Use the mixValue parameter to control whether to use brush texture to adjust the edge
    if( mixValue == 1)//使用mivValue参数来控制是否使用笔刷纹理修正边缘
    {
        if (color2.r < 0.05 && color2.g < 0.05 && color2.b < 0.05) {//focus on black edge
            color2 = finalColor.rgb - vec3(brushcolor.a);
        }
    }
    
    vec3 color3 = mix(color2, color2 * 0.5 + lighting2, lightStrength);

    

    // 计算表面体积
    float V_surface = curvature > 0.1 ? 1.0 : 0.0;

    // 计算k值
    float k;
    if (curvature <= 0.05) k = 0.0;
    else if (curvature <= 0.06) k = 0.5;
    else if (curvature <= 0.07) k = 0.8;
    //else if (curvature <= 0.2) k = 0.7;
    else k = 0.8;

    // 计算表面质量
    float rho_ink = 1.0; // 假设墨水密度为1.0
    float omega = 1.0; // 假设常数omega为1.0
    float m_surface = rho_ink * V_surface * pow(1.0 + omega / k, -1.0);

    // 使用表面质量m_surface进行渲染
    vec4 currentInk = texture(texture_diffuse1, gl_FragCoord.xy * texelSize);
    vec4 neighborInk = texture(texture_diffuse1, gl_FragCoord.xy * texelSize + texelSize * vec2(1.0, 0.0)) +
                       texture(texture_diffuse1, gl_FragCoord.xy * texelSize + texelSize * vec2(-1.0, 0.0)) +
                       texture(texture_diffuse1, gl_FragCoord.xy * texelSize + texelSize * vec2(0.0, 1.0)) +
                       texture(texture_diffuse1, gl_FragCoord.xy * texelSize + texelSize * vec2(0.0, -1.0));
    
    vec4 averageNeighborInk = neighborInk / 4.0;
    vec4 diffusedInk = mix(currentInk, averageNeighborInk, diffusionRate);

    // 将墨水属性转换为屏幕像素颜色值
    float C_i = m_surface; // 使用m_surface作为墨水浓度
    float C = 1.0 - pow(1.0 + 0.01 * C_i * C_i, -0.5);
    C = C * 1000 + 0.1;
    
    
    float m = (1- curvature)*(1- curvature);
    //FragColor = diffusedInk * m_surface; // 将扩散结果与表面质量相结合
    //FragColor = vec4(curvature,curvature,curvature,1.0);
    // 如果白色图是全白，可以直接设定颜色为白色
    vec4 whiteColor = vec4(1.0, 1.0, 1.0, 1.0);

    // 使用alpha混合，半透明图的alpha值决定最终颜色
    vec3 color4 = mix(whiteColor.rgb, color3.rgb, k);
    color4 = color3.rgb * 0.5 + color4*0.5;
   
    // 设置片段颜色
    FragColor = vec4(color3.rgb, 1.0); // 假设使用灰度颜色
    if(kEnabled == 1)
        FragColor = vec4(color4.rgb, 1.0); 
    
}

