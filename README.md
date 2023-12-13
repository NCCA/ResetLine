# Restart Line

This project demonstrates how to use [glPrimitiveRestartIndex](https://registry.khronos.org/OpenGL-Refpages/gl4/html/glPrimitiveRestartIndex.xhtml) to draw different lines of different lengths using the [ngl::SimpleIndexVAO](https://github.com/NCCA/NGL/blob/main/src/SimpleIndexVAO.cpp) class 

TODO

Simple points work next
1. Make blades just a ```std::vector<ngl::Vec3>```
2. randomly choose number of lines per blade
3. choose points
4. calculate max index for restart
5. generate VAO

Then perhaps generate a flow field for more fun later
    