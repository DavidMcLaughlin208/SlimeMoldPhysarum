// fragment shader

#version 440

out vec4 outputColor;
uniform int screenWidth;
uniform int screenHeight;
uniform float decay;
uniform float maxTrailDensity;
uniform float xRatio;
uniform float yRatio;
uniform vec4 teamColor1;
uniform vec4 teamColor2;
uniform vec4 teamColor3;
uniform vec4 teamColor4;
uniform vec4 baseColor;

struct Cell {
    vec4 val;
};

layout(binding = 2) buffer newTrail {
	Cell newTrailMap[];
};

void main()
{
    float x = gl_FragCoord.x / xRatio; 
    float y = gl_FragCoord.y / yRatio; 
    int index = int(x) + int(int(y) * screenWidth);
    
    vec4 val = newTrailMap[index].val / maxTrailDensity;
   // if (val.r > 0) {
   //     val.r += decay;  
   //} 
   //if (val.g > 0) {
   //     val.g += decay;
   //}
   //if (val.b > 0) {
   //     val.g += decay;
   //}
   //if (val.a > 0) {
   //     val.a += decay;
   //}

   float modR = max(0, val.r - val.g - val.b - val.a);
   float modG = max(0, val.g - val.r - val.b - val.a);
   float modB = max(0, val.b - val.g - val.r - val.a);
   float modA = max(0, val.a - val.g - val.b - val.r);

   vec4 chosenColor = baseColor;
   float weight = 1;
   if (val.r > val.g && val.r > val.b && val.r > val.a) {
        chosenColor = teamColor1;
        weight = modR;
   } else if (val.g > val.r && val.g > val.b && val.g > val.a) {
        chosenColor = teamColor2;
        weight = modG;
   } else if (val.b > val.r && val.b > val.g && val.b > val.a) {
        chosenColor = teamColor3;
       weight = modB;
   } else if (val.a > val.g && val.a > val.b && val.a > val.r) {
        chosenColor = teamColor4;
       weight = modA;
   }
   outputColor = mix(baseColor, chosenColor, weight + decay);
   //outputColor = vec4(colorR, colorG, 0, colorR + colorG);

}