# Ray-Tracing-v3

My 3rd attemp at a ray-tracer. Real-time with tens of thousands of triangles

![image](https://github.com/mariofvelez/Ray-Tracing-v3/assets/32421774/6fdc00e3-83ea-4367-a5ad-cacfa42b6423)
![image](https://github.com/mariofvelez/Ray-Tracing-v3/assets/32421774/1cc5000f-1f63-495b-ae52-a6faf046f977)

# Features
- Loading from OBJ files
- BVH acceleration
- Reflections
- Vertex normals and texturing

# What I Learned
- Ray-triangle intersection
- BVH construction and linearization on CPU and traversal on GPU
- loading OBJ files and storing them in a SSBO
- Barycentric coordinate system for interpolating normals and textures

# Things to Implement
- BVH construction on GPU for dynamic scenes
- PBR materials
- Emissive materials (lights)
- Multiple Importance Sampling
- Image-based materials
- Load scenes from USDZ file
- Switch to compute shaders instead of fragment shader
- Use of RT cores with DirectX 12
- Use of vertex normals

# Things to Fix
- [FIXED] Takes long to load application on my desktop and takes 3GB of memory to compile shader (???)
- [FIXED] Separate BVH building and linearization. This should fix the missing triangles problem
