#version 330 core
layout(location = 0) in vec3 aPos;

out vec4 VertexColor;

uniform vec4 BaseVertexColor;
uniform float WidthToHeightRatio = 1.f;
uniform mat4 ViewspaceMatrix;

void main()
{
  vec4 transformedPos = ViewspaceMatrix * vec4(aPos, 1.0);

  float tempY = transformedPos.y;
  transformedPos.y = transformedPos.z;
  transformedPos.z = transformedPos.x;
  transformedPos.x = tempY;
  
  float DepthRatio = 1.f / (1.f + transformedPos.z / 50.f);
  gl_Position = vec4(transformedPos.x / WidthToHeightRatio * DepthRatio, transformedPos.y * DepthRatio, transformedPos.z, 1.0);
  
  VertexColor = BaseVertexColor;
}
