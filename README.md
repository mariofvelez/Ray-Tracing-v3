# Ray-Tracing-v3

My 3rd attemp at a ray-tracer. Real-time with tens of thousands of triangles

![image](https://github.com/mariofvelez/Ray-Tracing-v3/assets/32421774/1cc5000f-1f63-495b-ae52-a6faf046f977)

# Features
- Loading from OBJ files
- BVH acceleration
- Reflections

# What I Learned
- Ray-triangle intersection
- BVH construction and linearization on CPU and traversal on GPU
- loading OBJ files and storing them in a SSBO

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
- Takes long to load application on my desktop and takes 3GB of memory to compile shader (???)
- Problem does not occur on my laptop with the same code (???)
- Separate BVH building and linearization. This should fix the missing triangles problem
