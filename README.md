# Physically-Based-Lens-Flare
Lens Flare Rendering with Ray-Tracing and Lens Data

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

![exp1](https://user-images.githubusercontent.com/65929274/147754174-c00ac29f-8663-4d5a-b15f-be6f7b0d443a.png)


![process3](https://user-images.githubusercontent.com/65929274/147752267-cd979149-b8ca-455e-9527-6689375d1d6f.png)


### Example
Nikon
![flare1](https://user-images.githubusercontent.com/65929274/147655774-db4b6e69-f62e-4125-8450-0f19dfd08407.gif)

Angenieux
![flare3](https://user-images.githubusercontent.com/65929274/147655981-d8a41630-9548-4dc1-bee4-f90a21b1f370.gif)

Canon
![flare2](https://user-images.githubusercontent.com/65929274/147655826-940d7d5e-9f33-4686-b129-73754c28b5ab.gif)