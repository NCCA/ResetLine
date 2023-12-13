#version 410 core
layout(lines) in;
layout(line_strip, max_vertices = 48) out;

uniform vec3 baseColour=vec3(0,0,0);
uniform vec3 tipColour=vec3(1,1,1);
out vec3 vertColour;

void main()
{

  float s=1.0/gl_in.length();
  float t=0.0;
  for(int i = 0; i<gl_in.length(); ++i)
  {
    vertColour = mix(baseColour,tipColour,i);
    gl_Position = gl_in[i].gl_Position;
    EmitVertex();
    t+=s;

  }
EndPrimitive();
}
