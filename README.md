# Physically-Based-Lens-Flare
Realtime Lens Flare Rendering with Ray-Tracing and Lens Data

![GIF](https://user-images.githubusercontent.com/65929274/148648293-9785c3dd-b1a0-480f-8e3f-1acc133e07cc.gif)

### Task
- [x] chromatic aberration
- [x] non-linear ghost deformation 
- [ ] performance optimization

### Graphics API
DirectX12

### Necessary External File / Code
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
![nikon](https://user-images.githubusercontent.com/65929274/148648214-903d1502-e855-458d-8479-55402c00e74f.png)

Angenieux
![angenieux](https://user-images.githubusercontent.com/65929274/148648219-39a4a3bd-c6b1-40f1-b03c-a8ab5414acbc.png)

Canon
![canon](https://user-images.githubusercontent.com/65929274/148648226-4e4da6a0-ec68-4f29-975c-6a2be0524b6e.png)