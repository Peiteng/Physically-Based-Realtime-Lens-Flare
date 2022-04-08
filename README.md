# Physically-Based-Realtime-Lens-Flare
Realtime Lens Flare Rendering with Ray-Tracing and Lens Data
![ExecuteImage](https://user-images.githubusercontent.com/65929274/149627276-29002228-32b9-4eba-a63d-7fe364ca2f22.png)
![画像2](https://user-images.githubusercontent.com/65929274/162211430-cbe2a0e7-023c-4ad5-a795-68d3d734976a.png)
![画像1](https://user-images.githubusercontent.com/65929274/162211262-ed3d804e-f458-433a-b858-1f202cfe3a76.png)
![スクリーンショット 2022-04-08 231220](https://user-images.githubusercontent.com/65929274/162453614-2a9c59bc-d01f-4599-acf3-e3b9e6d644a7.png)

![GIF](https://user-images.githubusercontent.com/65929274/149627485-21b8b988-0697-41ac-83c8-abd61ba2bb1e.gif)

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
![nikon](https://user-images.githubusercontent.com/65929274/149627168-2ec0d99b-0d12-40d8-b2e5-7ab559e0feb3.png)

Angenieux
![angenieux](https://user-images.githubusercontent.com/65929274/149627169-1f37fab5-96e6-4f1b-adfa-9f916a2efac9.png)

Canon
![canon](https://user-images.githubusercontent.com/65929274/149627173-9f5834f0-cced-4174-babb-d2b607c65a35.png)


