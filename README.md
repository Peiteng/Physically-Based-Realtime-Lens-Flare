# Physically-Based-Lens-Flare
Realtime Lens Flare Rendering with Ray-Tracing and Lens Data

necessary external file / code
- d3dx12.h
- dxcompiler.dll
- dxil.dll
- imgui

### Calculation Method of Wave Distribution
- (Star)Burst Pattern
Fraunhofer Diffraction
![burst](https://user-images.githubusercontent.com/65929274/147729251-25154030-9cef-4f07-8895-01fbd01f3513.png)

- Ghost/Ringing Pattern
Angular Spectrum Method
![ghost](https://user-images.githubusercontent.com/65929274/147729265-d0b010c4-6bce-4956-989b-2120d8b62912.png)

### Simulation Process
First, we construct optical interface such as lens. Then, we can use patent data of camera.
We can simulate lens flare exactly by use of these information.
![process1](https://user-images.githubusercontent.com/65929274/147752414-3aa92a54-e088-46e5-9dca-1020ba5be6b1.png)

Lens form is denoted by algebraic equation, so we don't need to specific structure such as BVH.
![process2](https://user-images.githubusercontent.com/65929274/147752156-ae8adbb9-dfac-4767-bb94-43a67b0b1589.png)

By executing tracing of bundle, we can calculate intensity of ray. For improving in quality, we take average of its surrounding neighbors.

<img src="https://render.githubusercontent.com/render/math?math=I=\frac{S_{\text{base}}}{\sum_{n=0}^{3}S_{n}}">


![process3](https://user-images.githubusercontent.com/65929274/147752267-cd979149-b8ca-455e-9527-6689375d1d6f.png)


### Example
Nikon
![nikon](https://user-images.githubusercontent.com/65929274/148566646-64e4e430-d3b5-4e91-b9d0-bb14d0376fd7.png)

Angenieux
![angenieux](https://user-images.githubusercontent.com/65929274/148566701-6e538d2d-0df0-4027-a9cb-19ac18c7a847.png)

Canon
![canon](https://user-images.githubusercontent.com/65929274/148566743-07f40451-f518-442b-858f-ada39f72556f.png)