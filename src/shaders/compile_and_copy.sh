glslangValidator -V diffuse.vert -o diffuse.vert.spv
glslangValidator -V diffuse.frag -o diffuse.frag.spv

glslangValidator -V specular.vert -o specular.vert.spv
glslangValidator -V specular.frag -o specular.frag.spv

glslangValidator -V wireframe.vert -o wireframe.vert.spv
glslangValidator -V wireframe.frag -o wireframe.frag.spv

glslangValidator -V toon.vert -o toon.vert.spv
glslangValidator -V toon.frag -o toon.frag.spv

glslangValidator -V ui.vert -o ui.vert.spv
glslangValidator -V ui.frag -o ui.frag.spv

glslangValidator -V skybox.vert -o skybox.vert.spv
glslangValidator -V skybox.frag -o skybox.frag.spv

glslangValidator -V normals_debug.vert -o normals_debug.vert.spv
glslangValidator -V normals_debug.frag -o normals_debug.frag.spv
glslangValidator -V normals_debug.geom -o normals_debug.geom.spv
cp *.spv ../../data/shaders/
cp *.spv ../../build/data/shaders/
