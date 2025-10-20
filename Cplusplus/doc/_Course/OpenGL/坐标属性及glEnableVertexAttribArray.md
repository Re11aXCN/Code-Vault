1. 顶点着色器 (Vertex Shader)
   顶点着色器是渲染管线的第一个阶段，用于处理每个顶点的位置和属性。它接收顶点数据作为输入，通过对顶点进行变换（如模型变换、视图变换和投影变换），将其转换到裁剪空间或标准化设备坐标系。

2. 片段着色器 (Fragment Shader)
   片段着色器在顶点着色器之后执行，负责计算每个像素的最终颜色输出。它可以使用顶点着色器传递的数据和纹理数据来确定像素的颜色，并应用光照、阴影、材质等效果。

3. 索引缓冲区 (Index Buffer)
   索引缓冲区存储了顶点数组中顶点的索引顺序。通过使用索引缓冲区，可以有效地重用顶点数据，减少内存占用并提高渲染效率。索引缓冲区指定了绘制顶点的顺序，避免了多次传递相同顶点数据的需要。

4. 顶点缓冲区 (Vertex Buffer)
   顶点缓冲区存储了顶点的数据，如位置、颜色、法线等。顶点数据通过顶点缓冲区传递给顶点着色器，供其进行处理和变换。顶点缓冲区可以包含多个顶点的数据，可以通过索引缓冲区进行索引访问。

5. 纹理 (Texture)
   纹理是图形渲染中常用的一种技术，用于给物体表面贴图或者应用其他视觉效果。纹理可以包含颜色信息、法线信息、环境光遮蔽信息等。纹理可以绑定到物体的几何形状上，在片段着色器中使用纹理坐标进行采样，从而确定每个像素的最终颜色。

6. 统一变量 (Uniforms)
   统一变量是顶点着色器和片段着色器之间的全局变量。它们的值在每次绘制调用时保持不变，用于传递渲染过程中需要的常量数据，如光照信息、投影矩阵、纹理样本等。统一变量通过 OpenGL API 设置和更新，能够在渲染过程中动态地影响着色器的计算结果。

7. 顶点布局 (Vertex Layout)
   顶点布局定义了顶点数据在顶点缓冲区中的存储格式和布局方式。它描述了顶点数据的每个属性的类型、数量、偏移量等信息，确保顶点数据正确地被顶点着色器解析和处理。
   渲染流程和关联

    顶点数据传递和处理： 顶点数据首先存储在顶点缓冲区中，顶点着色器通过顶点布局解析这些数据，并进行变换和处理。

    片段着色和纹理采样： 片段着色器使用顶点着色器传递的数据和纹理采样器从纹理中获取像素颜色值。

    统一变量传递： 统一变量用于在渲染过程中传递常量数据，如视图矩阵、光照设置等，确保渲染的一致性和效率。

    绘制调用和渲染管线执行： OpenGL API 调用将顶点数据、纹理数据、统一变量设置和着色器程序绑定到渲染管线上，执行绘制调用后，OpenGL 依据管线的设置和数据进行渲染，最终将结果输出到帧缓冲区中。



## 1. glEnableVertexAttribArray(0)和glEnableVertexAttribArray(1)含义

```cpp
// 坐标
	glEnableVertexAttribArray(0);
	// first:顶点属性的索引（或位置），second:三维，third:类型，forth:是否需要将数据归一化（即，映射到[0,1]或[-1,1]范围内），fifth:坐标间距，sixth:指向数组中第一个组件的指针
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

	// 颜色RGB
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // 线条显示
```



> 在您的顶点数据数组 `float vertices[]` 中，数据是按顺序存储的，每个顶点由位置和颜色两部分组成，每部分都是三个浮点数。因此，数组中的每六个浮点数代表一个顶点的完整信息（三个位置分量和三个颜色分量）。
>
> `glEnableVertexAttribArray(0);` 和 `glEnableVertexAttribArray(1);` 分别对应数组 `vertices[]` 中的位置和颜色数据。具体来说：
>
> - `glEnableVertexAttribArray(0);` 对应于顶点数据中的位置信息（即每个顶点的 x, y, z 坐标）。在 `vertices[]` 数组中，这些值位于每六个浮点数的前三个位置。例如，第一个顶点的位置是 `-0.5f, -0.5f, 0.0f`。
>
> - `glEnableVertexAttribArray(1);` 对应于顶点数据中的颜色信息（即每个顶点的红、绿、蓝分量）。在 `vertices[]` 数组中，这些值紧跟在每个顶点的位置信息之后，即每六个浮点数的后三个位置。例如，第一个顶点的颜色是 `1.0f, 0.0f, 0.0f`。
>
> 当您调用 `glVertexAttribPointer` 函数时，您为这两个顶点属性（位置和颜色）分别指定了如何从 `vertices[]` 数组中读取数据：
>
> - 对于位置属性（索引0），您指定了步长为 `6 * sizeof(float)`，因为每个顶点需要6个浮点数（3个位置分量和3个颜色分量）。同时，您指定了偏移量为 `(void*)0`，因为位置数据是从数组的开始处读取的。
>
> - 对于颜色属性（索引1），您同样指定了步长为 `6 * sizeof(float)`，但偏移量变为了 `(void*)(3 * sizeof(float))`，因为颜色数据紧跟在位置数据之后，每个顶点的位置数据占用3个浮点数的空间。
>
> 这样，OpenGL就知道如何从 `vertices[]` 数组中正确地读取位置和颜色数据，并将它们传递给顶点着色器中的相应输入变量。

## 2. 实现逻辑

> 在OpenGL中，`glEnableVertexAttribArray` 函数用于启用顶点属性数组。这是在使用顶点缓冲区对象（VBOs）或顶点数组（通过`glVertexAttribPointer`指定）进行渲染时非常重要的一个步骤。每个顶点属性（如位置、颜色、纹理坐标等）都需要在着色器中被引用，并且需要在OpenGL中明确启用，以便OpenGL知道哪些顶点属性是有效的，并且应该从绑定到当前上下文的缓冲区或内存中读取这些属性。
>
> 在你给出的代码片段中，你启用了两个顶点属性数组：
>
> 1. `glEnableVertexAttribArray(0);`
>    - 这行代码启用了索引为0的顶点属性数组。在你的顶点着色器中，你通过`layout (location = 0) in vec3 aPos;`定义了一个位置输入变量`aPos`，它的位置索引是0。因此，这行代码告诉OpenGL：“嘿，我将在顶点着色器中使用索引为0的顶点属性数组，并且你应该读取与之相关联的数据（在这里是顶点的位置数据）。”
>
> 2. `glEnableVertexAttribArray(1);`
>    - 类似地，这行代码启用了索引为1的顶点属性数组。在你的顶点着色器中，你通过`layout (location = 1) in vec3 aColor;`定义了一个颜色输入变量`aColor`，它的位置索引是1。这行代码告诉OpenGL：“同时，我也将在顶点着色器中使用索引为1的顶点属性数组，你应该读取与之相关联的数据（在这里是顶点的颜色数据）。”
>
> 紧接着，你使用`glVertexAttribPointer`函数为这两个顶点属性数组指定了具体的读取方式：
>
> - 对于位置数据（索引0），你指定了每个顶点有三个浮点数（`3`），数据类型为`GL_FLOAT`，不需要归一化（`GL_FALSE`），连续顶点之间的步长为`6 * sizeof(float)`（因为每个顶点包含位置和颜色信息，总共6个浮点数），以及指向第一个位置数据的指针（`(void*)0`）。
> - 对于颜色数据（索引1），你同样指定了每个顶点有三个浮点数（`3`），数据类型为`GL_FLOAT`，不需要归一化（`GL_FALSE`），但这次你通过`(void*)(3 * sizeof(float))`指定了偏移量，这是因为颜色数据紧跟在位置数据之后，每个位置数据占用3个浮点数（`3 * sizeof(float)`）的空间。
>
> 总之，`glEnableVertexAttribArray`的调用是告诉OpenGL哪些顶点属性是有效的，并且应该被读取和传递给着色器。没有这些调用，OpenGL将不会读取这些顶点属性数组，你的着色器将不会接收到正确的顶点数据，从而导致渲染结果不正确或根本不显示任何内容。

### 着色器

```cpp
// 在GPU上的数据
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 vertexColor;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0f);\n"
"   vertexColor = aColor;\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 vertexColor;\n"
//"uniform vec4 u_Color;\n" // CPU向GPU传数据——统一变量
"void main()\n"
"{\n"
"   FragColor = vec4(vertexColor, 1.0f);\n"
"}\0";
```

### 坐标属性

```cpp
// 坐标glEnableVertexAttribArray(0); 和 颜色glEnableVertexAttribArray(1);
float vertices[] = {
	-0.5f, -0.5f, 0.0f,		1.0f,0.0f,0.0f,
	 0.5f, -0.5f, 0.0f,		0.0f,1.0f,0.0f,
	 0.5f,  0.5f, 0.0f,		0.0f,0.0f,1.0f,
	-0.5f,  0.5f, 0.0f,		1.0f,1.0f,0.0f
};
```

