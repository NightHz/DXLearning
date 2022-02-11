# DirectX 学习代码

## [Direct3D 9](https://docs.microsoft.com/en-us/windows/win32/direct3d9/dx9-graphics)

### 固定管线渲染

使用 dx9 的固定管线渲染，我们可以实现**颜色绘制、光照模型、贴图绘制、颜色混合**，并可以利用**模板缓存**实现简单的**镜面**与**地面阴影**效果。

首先在一个窗口的基础上[创建一个 dx9 设备](DXLearning/dx9.cpp#L5-L58)。

要绘制一个 3D 物件，需要明确**绘制对象、摄像机、场景光源**这三要素。我们需要先设置好这些要素，再调用 `DrawPrimitive` 或 `DrawIndexedPrimitive` 完成绘制。

1. 绘制对象
   - [网格](DXLearning/dx9.cpp#L94-L108)
     - 顶点信息
       - 位置、法线、颜色、贴图坐标、...
     - 索引信息
   - [变换信息](DXLearning/dx9.cpp#L527)
   - [材质](DXLearning/dx9.cpp#L532)
   - [贴图](DXLearning/dx9.cpp#L537-L540)
2. 摄像机
   - [变换信息](DXLearning/dx9.cpp#L651-L663)
3. [场景光源](DXLearning/main.cpp#L122-L133)

如果要进行颜色混合，例如绘制透明物体，需要[启用混合并设置好混合因子](DXLearning/main.cpp#L140-L149)。

**最简单的绘制**

|线框|实体|
|:-:|:-:|
|![](img/dx9_1wireframe.png)|![](img/dx9_1solid.png)|

**颜色绘制**

![](img/dx9_2color.gif)

**光照模型**，光源为竖直向下的方向光

|光照|+高光|
|:-:|:-:|
|![](img/dx9_3light.gif)|![](img/dx9_3light_specular.gif)|

**贴图绘制**

|贴图|+高光|
|:-:|:-:|
|![](img/dx9_4texture.gif)|![](img/dx9_4texture_specular.gif)|

**颜色混合**，要按照远近顺序依次绘制

|透明物体|调整绘制顺序实现的一种透视效果|
|:-:|:-:|
|![](img/dx9_5alpha.gif)|![](img/dx9_5alpha_spec.gif)|

---

利用模板缓存可以实现一些特殊效果，绘制特殊效果时，一般按照以下流程

1. 通过绘制特殊物体[更新模板缓存](DXLearning/main.cpp#L310-L324)，来指定有效的绘制区域
2. 设置好特殊效果所需的[模板测试](DXLearning/main.cpp#L326-L344)，以及其他参数
3. 设置特殊效果所需的额外变换，可以附加在世界变换的后面或取景变换的前面，并进行[绘制](DXLearning/main.cpp#L346-L352)
4. [恢复之前的设置](DXLearning/main.cpp#L354-L360)

**镜面与地面阴影效果**

![](img/dx9_6stencile.png)

右侧白色平面为镜子，蓝色平面为地面。


