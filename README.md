# Ray-Tracing-v3

My 3rd attemp at a ray-tracer. an interactive Physically-Based Path Tracer

> Light source casting a shadow on a glossy dragon
![image](https://github.com/mariofvelez/Ray-Tracing-v3/assets/32421774/98f2f7df-0974-4c16-bca2-f99b2fac2e0a)

> Reflective metallic dragon on a glossy quad light by an environment map
![image](https://github.com/mariofvelez/Ray-Tracing-v3/assets/32421774/d54b5585-c9e8-4930-9506-1f32de3d92cd)

> Glossy dragon with metallic icosahedons on a glossy quad lit by an environment map
![image](https://github.com/mariofvelez/Ray-Tracing-v3/assets/32421774/6c88d2a5-959e-4bbd-8ce2-49814758fb8d)

> Visualization of the BVH. Notice how there is a larger triangle density near the head leading to a higher BVH node density. Boxes are colored based on the axis they were split by.
![image](https://github.com/mariofvelez/Ray-Tracing-v3/assets/32421774/3a917ddf-b39f-4a5f-b20d-db487217e944)

> Early demonstration of loading obj files
![image](https://github.com/mariofvelez/Ray-Tracing-v3/assets/32421774/1cc5000f-1f63-495b-ae52-a6faf046f977)

# Features
- Loading from OBJ and MTL files
- BVH acceleration
- Reflections
- Vertex normals and texturing

# What I Learned
- Ray-triangle intersection
- BVH construction and linearization on CPU and traversal on GPU
- loading OBJ and MTL files and storing them in a SSBO
- Barycentric coordinate system for interpolating normals and textures
- Reflectance models including diffuse and specular
- lighting using emissive materials and an environment map
- Monte Carlo integration with frame accumulation

# Things to Implement
- BVH construction on GPU for dynamic scenes
- PBR materials
- Emissive materials (lights) - done
- Multiple Importance Sampling
- Image-based materials
- Load scenes from USD or USDZ file
- Switch to compute shaders instead of fragment shader
- Use of RT cores with DirectX 12 (This will be the next project)
- Frame accumulation for path tracing - done

# Things to Fix
- [FIXED] Takes long to load application on my desktop and takes 3GB of memory to compile shader (???)
- [FIXED] Separate BVH building and linearization. This should fix the missing triangles problem
- Vertex normal interpolation sometimes causes the normals to face away from the camera leading to artifacts around the edges of objects
