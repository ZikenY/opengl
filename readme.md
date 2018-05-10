# OpenGL/GLSL course projects - May 2017
## tested on Ubuntu/gcc5.x and Windows/Visual C++2013.

## **Raytracing algorithm:**

![Alt text](raytrace/screenshot01.jpg?raw=true "")<br/>

### We care about rays that hit our eyes.<br/>
Fundamental steps of Raytracing:<br/>
1. Generate primary ray;<br/>
2. Trace each ray;<br/>
3. Compute incoming illumination with recursive rays - shading.<br/>

Basic Raytracing step-by-step:<br/>
1. Fire a single ray from each pixel position into the scence along the projection patch;<br/>
2. Determine which surfaces the ray intersects and order them by distance to pixel;<br/>
3. The nearest surface to the pixel is the visible one for that pixel;<br/>
4. Reflect a ray off the visible surface along the specular reflection angle;<br/>
6. Repeat the process for secondary, third, forth rays and so on...<br/>

<br/><br/>

## **Shadow-Mapping and Environment-Mapping:**

![Alt text](shadowmapping/screenshot01.jpg?raw=true "")<br/>

### shadow-mapping
A basic shadow-mapping algorithm consists in two parts. The first part renders from the point of view of the light and computes the depth of each fragment. The second part renders the scene as usual, but with an extra test for each fragment to see if it in the shadow or not. If a fragment is in the shadow, Its brightness will be decreased to a half value.<br/>

The scene consists of two objects, one is a mesh read from file, the other is a quad floor created in memory. A point light is placed very far away from the scene in order to simulate a parallel light source. The angle of the light source is carefully adjusted so that the shadow of the mesh object is projected on the floor, and it can be easily observed from the point of the view of the camera. The user can rotate, scale and translate the mesh object to observe the change of the shadow's shape.<br/>

One of a remaining problem is the serrated edges of the shadow. It can be mitigated by increasing the resolution of the shadow map. In my computer, edges are becoming smooth when the size of the shadow map is larger than 4096*4096. However, depending on the capability of the GPU, the size of a texture is limited.<br/>

BTW, The shadow-mapping implementation only works on Windows. In Ubuntu14.04 or 16.04, I was unable to get the correct values in the shadow buffer :(<br/>

### Environment-mapping
Render a big cube with texture and place the camera in the center. Let the box move with the camera, but not rotate with the camera.<br/>

Set the z-value in Z-buffer to -1.<br/>

<br/><br/>

## **Mash rendering & Trackball control:**

![Alt text](modelshader/screenshot01.jpg?raw=true "")<br/>

matrix4f.cpp implements the basic mathmatic/vector/matrix functions and structures. The trackball is implemented by quaternion.<br/>
glaux.cpp implements the common mouse/keyboard view control functions.<br/>
load_m.cpp implements a mash file loader.<br/>
