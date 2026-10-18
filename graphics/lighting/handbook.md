# 光照与材质学习手册

> **写在前面的话**
>
> 如果你已经理解了渲染管线、能写基础 Shader，但面对"怎么让画面好看"这个问题还是一头雾水——这份手册就是为你写的。
>
> 光照和材质是渲染中最核心的视觉部分。从最朴素的 Lambert 到工业级的 PBR，从写实渲染到二次元卡通渲染，这份手册会带你一步步走完。
>
> 每个光照模型都会讲清楚：**它解决什么问题、公式怎么来的、参数怎么调**。不是让你背公式，而是让你理解"为什么是这样"。
>
> 建议的学习节奏：前四章打基础（3-4 天），PBR 部分多花时间（5-7 天），阴影和环境光各 2-3 天，最后看卡通渲染。每章都建议动手写 Shader 跑一跑。

---

## 目录

- [第一章 经典光照模型：从 Lambert 到 Blinn-Phong](#第一章-经典光照模型从-lambert-到-blinn-phong)
- [第二章 PBR 基础：从物理出发看光照](#第二章-pbr-基础从物理出发看光照)
- [第三章 Cook-Torrance BRDF：PBR 的核心公式](#第三章-cook-torrance-brdfpbr-的核心公式)
- [第四章 材质系统：贴图与工作流](#第四章-材质系统贴图与工作流)
- [第五章 阴影：Shadow Map 从原理到工程](#第五章-阴影shadow-map-从原理到工程)
- [第六章 环境光照：IBL 基础](#第六章-环境光照ibl-基础)
- [第七章 卡通渲染：二次元的光照哲学](#第七章-卡通渲染二次元的光影哲学)
- [附录 光照参数速查表](#附录-光照参数速查表)

---

## 第一章 经典光照模型：从 Lambert 到 Blinn-Phong

### 1.1 先搞清楚：光照在算什么

所有光照模型都在回答同一个问题：

> **给定一个表面点，它的颜色有多亮？**

这个问题的答案取决于：
- 光源的性质（方向、颜色、强度）
- 表面的性质（颜色、朝向、光滑程度）
- 观察者的位置

经典光照模型把这个问题拆成两部分独立计算：

$$\text{最终颜色} = \text{环境光} + \text{漫反射} + \text{高光反射}$$

| 分量 | 物理含义 | 视觉效果 |
|------|----------|----------|
| 环境光 (Ambient) | 模拟间接光照的近似 | 让阴影不完全黑 |
| 漫反射 (Diffuse) | 光在表面下散射后均匀射出 | 决定物体的"固有色" |
| 高光反射 (Specular) | 光在表面上的镜面反射 | 决定物体的光泽感 |

```
                 高光反射（镜面方向附近）
                    ↑
                   /|\
                  / | \
    光源 →  ↘   /  |  \   → 观察者
              \ /   |   \
               ·────┼──── 表面
                    |
               漫反射（所有方向均匀散射）
```

### 1.2 Lambert 漫反射：最朴素的光照

#### 核心思想

粗糙的表面（如粉笔、布料）会把光均匀散射到所有方向。从哪个方向看都一样亮——但**越正对光源越亮**。

#### 公式

$$L_{\text{diffuse}} = k_d \cdot (\vec{N} \cdot \vec{L})$$

- $\vec{N}$：表面法线（单位向量）
- $\vec{L}$：指向光源的方向（单位向量）
- $k_d$：漫反射系数（物体的"固有色"）
- $\vec{N} \cdot \vec{L} = \cos\theta$：光越正对表面，值越大

```
       光源
         ↓ L⃗
         ↓
  ───────·─────── 表面
         ↑ N⃗ (法线)
         
  N⃗ · L⃗ = cos(θ)
  θ = 0° → cos = 1 → 最亮
  θ = 90° → cos = 0 → 全暗
```

#### Shader 实现

```glsl
// 输入
vec3 N = normalize(fragNormal);   // 法线
vec3 L = normalize(lightDir);     // 光方向
vec3 lightColor = vec3(1.0);      // 光源颜色
vec3 albedo = vec3(0.8, 0.2, 0.1); // 物体颜色

// Lambert 漫反射
float NdotL = max(dot(N, L), 0.0);  // 半包络，背面不受光
vec3 diffuse = albedo * lightColor * NdotL;

// 加上环境光
vec3 ambient = albedo * 0.1;  // 简单粗暴：固有色的 10%
vec3 finalColor = ambient + diffuse;
```

#### 问题

Lambert 只有漫反射，没有高光。所以它看起来**完全没有光泽**——像粉笔或泥巴。

### 1.3 Phong 反射模型：加入高光

#### 核心改进

在 Lambert 的基础上，加一个**高光分量**。高光出现在**反射方向附近**。

#### 思路

1. 计算光在表面上的**反射方向** $\vec{R}$
2. 看观察方向 $\vec{V}$ 和反射方向 $\vec{R}$ 有多接近
3. 越接近，高光越强

```
                R⃗ (反射方向)
               ↗
              / 
    光源 →  /  ● 观察者
           ↗   ↗
          /   ↗ V⃗ (观察方向)
  ───────·──────── 表面
         ↑ N⃗
         
  R⃗ = 2(N⃗ · L⃗)N⃗ - L⃗   (反射向量公式)
  高光 = (R⃗ · V⃗)^n
```

#### 公式

$$L_{\text{specular}} = k_s \cdot (\vec{R} \cdot \vec{V})^n$$

- $\vec{R} = 2(\vec{N} \cdot \vec{L})\vec{N} - \vec{L}$：反射方向
- $\vec{V}$：指向观察者的方向（单位向量）
- $k_s$：高光系数
- $n$：**光泽度指数**（shininess），$n$ 越大高光越小越锐利

```
n = 8 (粗糙)          n = 128 (光滑)
   ▁▂▃▄▅▆▇█▇▆▅▄▃▂▁      ▁▂█▂▁
   宽而矮                窄而高
```

#### 完整的 Phong 模型

$$L = k_a + k_d \max(\vec{N} \cdot \vec{L}, 0) + k_s (\vec{R} \cdot \vec{V})^n$$

#### Shader 实现

```glsl
vec3 N = normalize(fragNormal);
vec3 L = normalize(lightDir);
vec3 V = normalize(viewDir);

// 环境光
vec3 ambient = albedo * 0.1;

// 漫反射 (Lambert)
float diff = max(dot(N, L), 0.0);
vec3 diffuse = albedo * lightColor * diff;

// 高光 (Phong)
vec3 R = reflect(-L, N);            // 反射方向
float spec = pow(max(dot(R, V), 0.0), 32.0);  // n = 32
vec3 specular = vec3(0.5) * spec;   // 高光颜色

vec3 finalColor = ambient + diffuse + specular;
```

#### 问题

Phong 模型需要计算反射向量 `reflect(-L, N)`，这个计算有点贵（一次点积、一次乘加）。而且在某些角度，高光会出现**不连续的断裂**（因为反射方向在法线变化时可能突变）。

### 1.4 Blinn-Phong：更聪明的做法

#### 核心改进

不计算反射方向 $\vec{R}$，而是计算**半程向量** $\vec{H}$：

$$\vec{H} = \frac{\vec{L} + \vec{V}}{\|\vec{L} + \vec{V}\|}$$

然后用 $\vec{N} \cdot \vec{H}$ 代替 $\vec{R} \cdot \vec{V}$。

```
                V⃗ (观察方向)
               ↗
              / 
    光源 →  /  ● 观察者
           ↗   ↗
          /   ↗ 
  ───────·──────── 表面
         ↑ N⃗    ↗
                H⃗ = normalize(L⃗ + V⃗)
                (半程向量)
                
  Blinn-Phong 高光 = (N⃗ · H⃗)^n
```

#### 为什么更好

| 对比项 | Phong | Blinn-Phong |
|--------|-------|-------------|
| 计算量 | 需要 `reflect` + 一次 `dot` | 只需一次 `normalize` + 一次 `dot` |
| 高光形状 | 略椭圆 | 更圆，更自然 |
| 断裂问题 | 有 | 基本没有 |
| 视觉效果 | 几乎一样 | 几乎一样 |

#### 公式

$$L_{\text{specular}} = k_s \cdot (\vec{N} \cdot \vec{H})^n$$

#### Shader 实现

```glsl
vec3 N = normalize(fragNormal);
vec3 L = normalize(lightDir);
vec3 V = normalize(viewDir);

// 环境光
vec3 ambient = albedo * 0.1;

// 漫反射
float diff = max(dot(N, L), 0.0);
vec3 diffuse = albedo * lightColor * diff;

// 高光 (Blinn-Phong)
vec3 H = normalize(L + V);                    // 半程向量
float spec = pow(max(dot(N, H), 0.0), 64.0);  // n = 64
vec3 specular = vec3(0.5) * spec;

vec3 finalColor = ambient + diffuse + specular;
```

### 1.5 三者对比

```
Lambert：                 Phong：                   Blinn-Phong：
只有漫反射                + 高光（反射方向）         + 高光（半程向量）
像粉笔                    像塑料                     像塑料/陶瓷
无光泽                    有光泽                     有光泽
最便宜                    中等                       最便宜（且效果好）
```

### 1.6 多光源叠加

多个光源时，每个光源独立计算，最后叠加：

```glsl
vec3 result = vec3(0.0);

for (int i = 0; i < lightCount; i++) {
    vec3 L = normalize(lights[i].position - fragPos);
    float dist = length(lights[i].position - fragPos);
    float attenuation = 1.0 / (dist * dist);  // 距离衰减
    
    vec3 diffuse = albedo * lights[i].color * max(dot(N, L), 0.0);
    vec3 H = normalize(L + V);
    vec3 specular = vec3(0.5) * pow(max(dot(N, H), 0.0), 64.0);
    
    result += (diffuse + specular) * attenuation;
}

result += ambient;
```

### 1.7 衰减（Attenuation）

真实世界中，光强度随距离的平方衰减：

$$I = \frac{I_0}{d^2}$$

但 $1/d^2$ 在 $d$ 很小时趋向无穷大，所以游戏里通常用：

$$I = \frac{I_0}{k_c + k_l \cdot d + k_q \cdot d^2}$$

| 系数 | 名称 | 作用 | 典型值 |
|------|------|------|--------|
| $k_c$ | 常数项 | 防止除零 | 1.0 |
| $k_l$ | 一次项 | 线性衰减 | 0.01~0.1 |
| $k_q$ | 二次项 | 平方衰减 | 0.001~0.01 |

---

## 第二章 PBR 基础：从物理出发看光照

### 2.1 经典模型的问题

Blinn-Phong 虽然好用，但它有几个根本性的问题：

| 问题 | 表现 |
|------|------|
| 不遵守能量守恒 | 高光 + 漫反射可能比入射光还亮 |
| 菲涅尔效应缺失 | 掠射角应该更像镜子，但 Blinn-Phong 不会 |
| 粗糙度表现不真实 | 粗糙表面的高光应该更大更暗，但指数调整效果不自然 |
| 材质不统一 | 金属和非金属需要完全不同的参数调法 |

PBR（Physically Based Rendering，基于物理的渲染）就是为了解决这些问题。

### 2.2 PBR 的三大原则

| 原则 | 含义 |
|------|------|
| **基于微表面理论** | 所有表面由无数微小的镜面组成 |
| **能量守恒** | 反射光的总能量 ≤ 入射光能量 |
| **使用基于物理的 BRDF** | 用物理推导的公式代替经验公式 |

### 2.3 微表面理论（Microfacet Theory）

#### 核心思想

宏观上看起来"粗糙"的表面，放大看其实是无数朝向不同的**微小镜面**：

```
光滑表面（如镜子）：         粗糙表面（如塑料）：

  ↗↗↗↗↗↗↗↗↗↗               ↗ ↗↗   ↗↗
  微面元朝向一致               ↗  ↗↗ ↗   ↗
  反射方向集中                 ↗  ↗  ↗↗ ↗
                               微面元朝向杂乱
                               反射方向分散
```

- 表面越**光滑**：微面元朝向越一致，反射越集中，高光越小越亮
- 表面越**粗糙**：微面元朝向越分散，反射越分散，高光越大越暗

这就是 PBR 中**粗糙度（Roughness）**参数的物理含义。

### 2.4 BRDF 是什么

**BRDF（Bidirectional Reflectance Distribution Function，双向反射分布函数）** 描述的是：

> 给定一个入射光方向 $\omega_i$，一个出射（观察）方向 $\omega_o$，表面反射的光占总入射光的比例是多少？

$$f_r(\omega_i, \omega_o) = \frac{dL_o(\omega_o)}{L_i(\omega_i) \cos\theta_i \, d\omega_i}$$

不用被公式吓到。直觉理解：

```
              ω_o (出射方向/观察方向)
             ↗
            /
    ───────·────── 表面
           ↑
          ω_i (入射方向/光方向)

BRDF(ω_i, ω_o) = "从 ω_i 方向来的光，有多少被反射到 ω_o 方向"
```

### 2.5 BRDF 的两个分量

PBR 中常用的 BRDF 分为两个分量：

$$f_r = f_{\text{diffuse}} + f_{\text{specular}}$$

| 分量 | 物理含义 | 模型 |
|------|----------|------|
| 漫反射 BRDF | 光进入表面下，散射后射出 | Lambert（简单够用） |
| 高光 BRDF | 光在微表面上的镜面反射 | Cook-Torrance（下一章讲） |

### 2.6 渲染方程

BRDF 是局部模型（只考虑直接光照）。完整的渲染还要考虑**间接光**（环境光、其他物体的反射光）。这由**渲染方程**描述：

$$L_o(\omega_o) = L_e(\omega_o) + \int_{\Omega} f_r(\omega_i, \omega_o) \cdot L_i(\omega_i) \cdot (\vec{N} \cdot \omega_i) \, d\omega_i$$

- $L_o$：出射光（你看到的颜色）
- $L_e$：自发光
- 积分项：所有入射方向的光经 BRDF 反射后的总和

这个积分在实时渲染中无法精确求解，所以需要各种近似——这就是 PBR 管线中各种预计算和近似技巧的由来。

### 2.7 能量守恒

**反射光的总能量不能超过入射光。**

直觉：一个表面反射了 30% 的光作为高光，那漫反射最多只能用 70%。

```
入射光 = 100%
  ├── 高光反射 = 30%（菲涅尔决定）
  └── 漫反射 ≤ 70%（剩余能量）
```

在 Shader 中的体现：

```glsl
vec3 kS = fresnelSchlick(dot(H, V), F0);  // 高光占比
vec3 kD = vec3(1.0) - kS;                  // 漫反射占比
kD *= (1.0 - metallic);                    // 金属没有漫反射

vec3 diffuse = kD * albedo / PI * NdotL;   // 漫反射
vec3 specular = kS * D * G / (4.0 * NdotV * NdotL + 0.0001);  // 高光
```

### 2.8 菲涅尔效应（Fresnel Effect）

#### 现象

看一个湖面：正下方能看到水底（漫反射为主），远处的水面更像一面镜子（高光反射为主）。

这就是**菲涅尔效应**：**观察角度越接近掠射角（grazing angle），反射越强。**

```
  正上方看（近法线方向）：    远处看（掠射角）：
  反射弱，看到水下           反射强，水面像镜子
  
      ↓                          ↗↗↗
     ~~~                        ~~~~~
    看到水底                    看到天空的反射
```

#### 菲涅尔方程（Schlick 近似）

精确的菲涅尔方程很复杂，Schlick 给出了一个简单又好用的近似：

$$F(\theta) = F_0 + (1 - F_0)(1 - \cos\theta)^5$$

- $F_0$：垂直入射时的反射率（材质属性）
- $\theta$：入射角（光和法线的夹角）

```glsl
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
```

#### $F_0$ 的值

| 材质 | $F_0$ | 说明 |
|------|-------|------|
| 非金属（皮肤、塑料、石头） | 0.02~0.05 | 反射很弱 |
| 水 | 0.02 | |
| 钻石 | 0.17 | |
| 金属（铁、铜、金、铝） | 0.5~1.0 | 反射很强，且带颜色 |

> **关键认知**：非金属的 $F_0$ 几乎都是 0.04 左右，变化不大。金属的 $F_0$ 则因材质不同差异很大，而且**有颜色**（金是黄色，铜是橙色）。

---

## 第三章 Cook-Torrance BRDF：PBR 的核心公式

### 3.1 总览

Cook-Torrance 模型是 PBR 中最常用的高光 BRDF：

$$f_{\text{specular}} = \frac{D \cdot F \cdot G}{4 (\vec{N} \cdot \vec{V})(\vec{N} \cdot \vec{L})}$$

三个分量各有物理含义：

| 分量 | 名称 | 含义 |
|------|------|------|
| $D$ | 法线分布函数 (NDF) | 微面元朝向的统计分布 |
| $F$ | 菲涅尔项 | 反射比例随角度变化 |
| $G$ | 几何函数 | 微面元之间的遮蔽和阴影 |

```
微表面模型示意：

     光线 →  ↘       ↙ ← 观察方向
              \     /
    微面元 →   · · ·    ← 有些微面元朝向恰好是半程向量 H
                · ·
  ────────── 宏观表面 ──────────
  
  D: 有多少微面元朝向 H 方向？
  F: 到达表面的光有多少被反射（vs 折射）？
  G: 微面元之间有没有互相遮挡？
```

### 3.2 法线分布函数 D（Normal Distribution Function）

#### 作用

估算微面元中，法线方向恰好等于半程向量 $\vec{H}$ 的比例。

#### GGX / Trowbridge-Reitz 模型

游戏引擎中最常用的 NDF：

$$D_{\text{GGX}} = \frac{\alpha^2}{\pi ((\vec{N} \cdot \vec{H})^2 (\alpha^2 - 1) + 1)^2}$$

其中 $\alpha$ 是粗糙度参数（通常 $\alpha = \text{roughness}^2$，平方是为了让美术调参更线性）。

```
roughness = 0.2 (光滑)        roughness = 0.8 (粗糙)
      ▁▂▃▄▅▆▇██▇▆▅▄▃▂▁           ▁▂▃▄▅▄▃▂▁
      窄而高                       宽而矮
      高光集中                     高光分散
```

```glsl
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;       // α = roughness²
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;
    
    return a2 / max(denom, 0.0001);
}
```

### 3.3 几何函数 G（Geometry Function）

#### 作用

描述微面元之间**互相遮蔽**的现象。粗糙表面上的"凹坑"会让一些微面元被其他微面元挡住。

分为两部分：
- **Shadowing**：微面元被其他微面元遮挡，照不到光
- **Visibility**：微面元反射的光被其他微面元挡住，到不了观察者

```
     光线 →  ↘
              \
    微面元 →   ·×·    ← × 表示被遮挡，照不到光
                ··
  ────────────────────
  
  粗糙度越高 → 遮蔽越严重 → G 越小 → 高光越暗
```

#### Smith 方法 + Schlick-GGX

```glsl
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;   // 直接光照的 k
    // 如果是环境光照（IBL），k = roughness² / 2
    
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);  // 观察方向
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);  // 光照方向
    return ggx1 * ggx2;
}
```

### 3.4 菲涅尔项 F

已经在上一章讲过，用 Schlick 近似：

```glsl
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
```

### 3.5 完整的 PBR 光照 Shader

```glsl
// ===== 输入 =====
in vec3 fragPos;
in vec3 fragNormal;
in vec2 fragUV;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

uniform sampler2D albedoMap;    // 基础色
uniform sampler2D metallicMap;  // 金属度
uniform sampler2D roughnessMap; // 粗糙度
uniform sampler2D normalMap;    // 法线贴图
uniform sampler2D aoMap;        // 环境光遮蔽

const float PI = 3.14159265359;

// ===== D: GGX 法线分布 =====
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

// ===== G: 几何函数 =====
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

// ===== F: 菲涅尔 =====
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    // 1. 采样材质
    vec3 albedo = texture(albedoMap, fragUV).rgb;
    float metallic = texture(metallicMap, fragUV).r;
    float roughness = texture(roughnessMap, fragUV).r;
    float ao = texture(aoMap, fragUV).r;
    
    // 2. 计算方向向量
    vec3 N = normalize(fragNormal);  // 实际应该从法线贴图计算
    vec3 V = normalize(viewPos - fragPos);
    vec3 L = normalize(lightPos - fragPos);
    vec3 H = normalize(V + L);
    
    // 3. 计算 F0（非金属默认 0.04，金属用 albedo）
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    
    // 4. Cook-Torrance 高光
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 numerator = D * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    // 5. 能量守恒：漫反射 = 1 - 高光占比
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= (1.0 - metallic);  // 金属没有漫反射
    
    // 6. 漫反射
    float NdotL = max(dot(N, L), 0.0);
    vec3 diffuse = kD * albedo / PI;
    
    // 7. 最终颜色
    vec3 Lo = (diffuse + specular) * lightColor * NdotL;
    
    // 8. 加上环境光（简化版）
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;
    
    // 9. HDR 色调映射
    color = color / (color + vec3(1.0));  // Reinhard
    
    // 10. Gamma 校正
    color = pow(color, vec3(1.0 / 2.2));
    
    outColor = vec4(color, 1.0);
}
```

### 3.6 典型参数范围

| 参数 | 非金属 | 金属 | 说明 |
|------|--------|------|------|
| albedo | sRGB 0.02~0.9 | sRGB 0.5~1.0 | 非金属不要全黑或全白 |
| metallic | 0.0 | 1.0 | 通常只有纯 0 或纯 1，中间值用于过渡 |
| roughness | 0.04~0.9 | 0.0~0.5 | 0 = 完美镜面，1 = 完全粗糙 |
| $F_0$ 非金属 | 0.02~0.05 | — | 几乎所有非金属都接近 0.04 |
| $F_0$ 金属 | — | (0.5~1.0, 带颜色) | 金=(1.0, 0.71, 0.29), 铜=(0.95, 0.64, 0.54) |

---

## 第四章 材质系统：贴图与工作流

### 4.1 PBR 材质贴图一览

在金属度/粗糙度工作流中，一个完整的 PBR 材质通常包含以下贴图：

| 贴图 | 通道 | 含义 | 值范围 |
|------|------|------|--------|
| **Base Color（基础色）** | RGB | 非金属=固有色，金属=反射颜色 | sRGB 0.02~0.9 |
| **Metallic（金属度）** | R | 0=非金属，1=金属 | 通常只有 0 或 1 |
| **Roughness（粗糙度）** | R | 0=镜面，1=完全粗糙 | 0.04~0.9 |
| **Normal（法线）** | RGB | 切线空间法线偏移 | 蓝色为主（z 正向） |
| **AO（环境光遮蔽）** | R | 缝隙/褶皱处的阴影 | 0~1，白色=无遮蔽 |
| **Emissive（自发光）** | RGB | 自发光颜色 | HDR 值 |
| **Height（高度）** | R | 视差贴图/曲面细分 | 灰度 |

### 4.2 各贴图详解

#### Base Color（基础色 / Albedo）

**作用**：定义表面的"固有色"——不考虑光照时的颜色。

**制作要点**：
- 非金属：sRGB 值在 30~240 之间（不要纯黑或纯白）
- 金属：代表反射颜色（金=黄色，铜=橙色）
- **不要包含光影信息**（那是 AO 贴图的工作）

```
好的基础色：               坏的基础色：
  颜色均匀                   有明暗变化（那是 AO 的事）
  没有高光                   有高光（那是粗糙度的事）
  细节来自纹理               有污渍/水渍（除非是特殊材质）
```

#### Metallic（金属度）

**作用**：区分金属和非金属区域。

**制作要点**：
- 通常只有纯黑（0）和纯白（1）
- 中间值只用于边缘过渡（如油漆剥落露出金属）
- 金属没有漫反射，所有颜色都来自镜面反射

```
铁质盔甲：
  盔甲表面（有漆）：metallic = 0
  磨损边缘（露出铁）：metallic = 1
  过渡区域：0~1 的渐变
```

#### Roughness（粗糙度）

**作用**：控制表面的光滑程度，直接影响高光的大小和锐利度。

**制作要点**：
- 0.04：最光滑（镜面、抛光金属），实际中几乎不存在完美光滑
- 0.2~0.4：光滑（湿地板、新车漆）
- 0.5~0.7：中等（木头、混凝土）
- 0.8~1.0：粗糙（粗布、生锈金属）

```
roughness = 0.1：     roughness = 0.5：     roughness = 0.9：
  ▁▂▃▄▅▆▇██▇▆▅▄▃▂▁      ▁▂▃▄▅▄▃▂▁            ▁▂▃▂▁
  锐利的小高光            中等大小               大面积柔和高光
```

#### Normal Map（法线贴图）

**作用**：在不增加几何复杂度的情况下，给表面添加细节。

**原理**：每个像素存储一个切线空间法线 $(x, y, z)$，映射到颜色 $(r, g, b)$：

$$\text{颜色} = \frac{\text{法线} + 1}{2}$$

所以法线贴图看起来偏蓝——因为 $z$ 分量（垂直于表面的方向）通常是正的。

**制作要点**：
- 从高精度模型"烘焙"到低精度模型
- 也可以用程序生成（如噪声）
- 强度可以通过一个参数控制

```glsl
// 法线贴图采样和应用
vec3 normalMap = texture(normalMap, fragUV).rgb;
vec3 tangentNormal = normalMap * 2.0 - 1.0;  // [0,1] → [-1,1]

// 可以调整强度
float normalStrength = 0.5;
tangentNormal.xy *= normalStrength;
tangentNormal = normalize(tangentNormal);

// 从切线空间变换到世界空间
vec3 worldNormal = normalize(TBN * tangentNormal);
```

#### AO Map（环境光遮蔽贴图）

**作用**：模拟缝隙、褶皱、角落处因为被遮挡而变暗的效果。

**原理**：衡量一个点能被多少方向的环境光照到。缝隙中的点只能被很少的方向照到，所以更暗。

```
  平坦区域：               缝隙中：
  ← ← ← ← ←              → ← ←
  ← ← ● ← ←              → ← ●
  ← ← ← ← ←              → ← ←
  能被所有方向照到          只能被少数方向照到
  AO = 1.0（白色）         AO = 0.3（暗色）
```

**使用方式**：

```glsl
float ao = texture(aoMap, fragUV).r;
// AO 只影响间接光（环境光），不影响直接光
vec3 ambient = ambientLight * albedo * ao;
```

### 4.3 贴图打包优化

实际引擎中，为了节省带宽，经常把多个灰度贴图打包到一个 RGBA 纹理中：

```
ORM 贴图（常见打包方式）：
  R 通道：AO
  G 通道：Roughness
  B 通道：Metallic
  A 通道：不用（或存其他数据）

好处：只需采样一次纹理，就能获取三个材质属性
```

### 4.4 材质实例与工作流

```
美术工作流：
  1. 在 Substance Painter / Mari 中绘制贴图
  2. 导出 BaseColor / Metallic / Roughness / Normal / AO
  3. 导入引擎，创建材质实例
  4. 调整参数（法线强度、UV 平铺等）
  5. 应用到模型

引擎中的材质系统：
  材质模板（Material Template）
    ├── 参数：baseColor, metallic, roughness, normalStrength ...
    ├── Shader 代码：PBR 光照计算
    └── 材质实例（Material Instance）
         ├── 实例 A：铁甲（metallic=1, roughness=0.4）
         ├── 实例 B：布料（metallic=0, roughness=0.8）
         └── 实例 C：皮肤（metallic=0, roughness=0.5, 加 SSS）
```

---

## 第五章 阴影：Shadow Map 从原理到工程

### 5.1 为什么阴影这么难

直接光照计算中，每个表面点只需要知道"有没有光到达我"。但"有没有被其他物体挡住"这个问题，需要从**光源的视角**来看——这就是 Shadow Map 的核心思想。

### 5.2 Shadow Map 基本原理

#### 两步走

```
第一步：从光源视角渲染深度图（Shadow Map）
  光源 → 渲染场景 → 得到每个像素到光源的距离
  
第二步：从摄像机视角渲染时，比较深度
  对每个片元：
    1. 计算它到光源的距离 d_scene
    2. 查 Shadow Map，得到该方向上最近的距离 d_shadow
    3. 如果 d_scene > d_shadow + bias → 在阴影中
```

```
第一步：                          第二步：
从光源看：                        从摄像机看：
                                  
  光源                             摄像机
   ↓                               ↓
   ·─── A (近)                     ·─── A
   |                                |
   ·─── B (远，在阴影中)            ·─── B → 查 Shadow Map
   |                                |       d_scene > d_shadow
   ·─── C                           ·─── C → 在阴影中
                                   
Shadow Map:                        对每个像素：
  A: 5m                             比较 d_scene 和 d_shadow
  B: 10m                            判断是否在阴影中
  C: 15m
```

### 5.3 实现步骤

#### 第一步：生成 Shadow Map

```glsl
// ===== Shadow Map 生成 Pass =====
// 顶点着色器：从光源视角变换
layout(location = 0) in vec3 position;

uniform mat4 lightSpaceMatrix;  // 光源的 VP 矩阵

void main() {
    gl_Position = lightSpaceMatrix * vec4(position, 1.0);
}

// 片元着色器：不需要做任何事，深度自动写入
// （可以不写片元着色器）
```

```cpp
// 渲染到 Shadow Map 纹理
VkRenderPassBeginInfo renderPassInfo = {};
renderPassInfo.framebuffer = shadowFramebuffer;
// ... 配置 shadowMapImage 作为深度附件

vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, ...);
// 绘制所有物体
for (auto& mesh : sceneMeshes) {
    drawMesh(mesh, lightSpaceMatrix);
}
vkCmdEndRenderPass(commandBuffer);
```

#### 第二步：使用 Shadow Map

```glsl
// ===== 主渲染 Pass =====
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;

float shadowCalculation(vec3 fragPos, vec3 normal) {
    // 1. 把片元位置变换到光源空间
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // 2. 变换到 [0, 1] 范围（Shadow Map 的 UV 范围）
    projCoords = projCoords * 0.5 + 0.5;
    
    // 3. 当前片元到光源的深度
    float currentDepth = projCoords.z;
    
    // 4. 查 Shadow Map
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    // 5. 比较（加 bias 防止阴影失真）
    float bias = 0.005;
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    
    return shadow;  // 0 = 被照亮, 1 = 在阴影中
}

void main() {
    float shadow = shadowCalculation(fragPos, fragNormal);
    vec3 lightColor = vec3(1.0);
    
    // 阴影中的光强减半
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = lightColor * diff * (1.0 - shadow * 0.5);
    
    // ... 其余光照计算
}
```

### 5.4 阴影失真（Shadow Acne）

#### 问题

Shadow Map 的分辨率有限，一个像素对应一个深度值。当表面和光线几乎平行时，多个表面片元共享同一个 Shadow Map 像素，导致深度比较出错——表面自己遮挡了自己。

```
Shadow Map 的一个像素覆盖了这段表面：

  光线 →  ╲╲╲╲╲╲╲╲╲
           ╲ ╲ ╲ ╲ ╲
  表面 ──────·─·─·─·──────
              ↑
         这一段在 Shadow Map 中
         共享同一个深度值
         → 部分片元误判为在阴影中
```

#### 解决方案：深度偏移（Depth Bias）

```glsl
// 方法一：固定偏移
float bias = 0.005;  // 简单但不够精确

// 方法二：基于斜率的偏移（更好）
float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
// 表面越平行于光线 → bias 越大
```

在 Vulkan 中，也可以在光栅化状态中设置多边形偏移：

```cpp
rasterizer.depthBiasEnable = VK_TRUE;
rasterizer.depthBiasConstantFactor = 1.25f;
rasterizer.depthBiasSlopeFactor = 1.75f;
```

### 5.5 Peter Panning（阴影悬浮）

#### 问题

bias 太大，阴影会"飘"在物体上方，和物体脱离。

```
正确的阴影：              Peter Panning：
  ████                      ████
  ████▓▓▓▓                  ████
  ▓▓▓▓                      ▓▓▓▓
  阴影紧贴物体               阴影和物体之间有间隙
```

#### 权衡

**Acne 和 Peter Panning 是一对矛盾**：
- bias 太小 → Acne（阴影自遮挡）
- bias 太大 → Peter Panning（阴影悬浮）

**解决方案**：
- 使用 **Normal Offset Bias**：沿法线方向偏移，而不是沿光线方向
- 使用 **Cascaded Shadow Map**：近处用高分辨率 + 小 bias

```glsl
// Normal Offset Bias
vec3 biasOffset = normal * bias;
vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPos + biasOffset, 1.0);
```

### 5.6 PCF 软阴影（Percentage Closer Filtering）

#### 问题

Shadow Map 的硬边缘不真实。真实世界的阴影有**半影区**（penumbra）。

#### 解决

对 Shadow Map 做多次采样，取平均值：

```glsl
float shadowPCF(vec3 fragPos, vec3 normal) {
    vec4 fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    float currentDepth = projCoords.z;
    float bias = 0.005;
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    // 3×3 采样
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float closestDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    return shadow;
}
```

```
硬阴影（1 次采样）：         软阴影（9 次采样 PCF）：
  ████▓▓▓▓                    ████▓▓▓▓
  ████▓▓▓▓                    ████▒▒▓▓
  边缘锐利                    边缘柔和过渡
```

### 5.7 级联阴影（Cascaded Shadow Map, CSM）

#### 问题

Shadow Map 的分辨率是固定的。近处的阴影需要高精度，远处的阴影可以粗糙一些。一张固定分辨率的 Shadow Map 无法兼顾。

#### 解决

用**多张 Shadow Map**，分别覆盖不同距离范围：

```
俯视图：

  摄像机 →
  
  ├── Cascade 0 ──┤── Cascade 1 ──┤── Cascade 2 ──┤── Cascade 3 ──┤
  0m             20m            50m           100m           500m
  
  分辨率：          分辨率：        分辨率：       分辨率：
  2048×2048        1024×1024      512×512       256×256
  （近处高精度）     （中等）        （较远）      （远处低精度）
```

#### 实现思路

```glsl
// 选择使用哪个 Cascade
int getCascadeIndex(float depth) {
    if (depth < 20.0) return 0;
    if (depth < 50.0) return 1;
    if (depth < 100.0) return 2;
    return 3;
}

// 在对应的 Cascade 中采样阴影
float getShadow(vec3 fragPos, vec3 normal, float viewDepth) {
    int cascade = getCascadeIndex(viewDepth);
    mat4 lightMatrix = cascadeMatrices[cascade];
    sampler2D shadowTex = cascadeMaps[cascade];
    // ... 和单张 Shadow Map 一样的采样逻辑
}
```

#### 在引擎中的典型配置

| 参数 | 典型值 | 说明 |
|------|--------|------|
| Cascade 数量 | 3~4 | 越多越精细，开销越大 |
| 最大距离 | 100~500m | 超过这个距离不画阴影 |
| 分辨率 | 2048² 或 4096² | 总分辨率，分配给各 Cascade |
| 混合区域 | Cascade 边界的 10% | 在 Cascade 之间平滑过渡 |

---

## 第六章 环境光照：IBL 基础

### 6.1 为什么需要环境光

前面讲的光照都是**直接光照**（点光源、方向光等）。但真实世界中，光会从各个方向照过来——天空、地面、周围物体的反射。这些**间接光照**对画面真实感至关重要。

最简单的环境光：

```glsl
vec3 ambient = vec3(0.03) * albedo;  // 所有地方均匀照亮
```

这太粗糙了。更好的方案是 **IBL（Image-Based Lighting）**。

### 6.2 IBL 的基本思路

用一张**环境贴图**（Environment Map / HDR Skybox）来表示整个环境的光照：

```
环境贴图（HDR 全景图）：
  
  ┌────────────────────────────────┐
  │         天空（亮）              │
  │    ☀                          │
  │                                │
  │ ───────── 地平线 ──────────── │
  │                                │
  │         地面（暗）              │
  └────────────────────────────────┘
  
  每个方向都有一个颜色值（HDR）
  这就是 "环境光" 的来源
```

### 6.3 漫反射 IBL：辐照度贴图（Irradiance Map）

#### 思路

漫反射是均匀的，所以对半球做积分：

$$L_{\text{diffuse}} = \frac{k_d \cdot \text{albedo}}{\pi} \int_{\Omega} L_i(\omega_i) (\vec{N} \cdot \omega_i) \, d\omega_i$$

这个积分的结果只和**法线方向**有关。所以可以预计算：对每个可能的法线方向，算出积分结果，存成一张**辐照度贴图**。

```
预计算：
  对每个法线方向 N⃗：
    积分 = Σ (环境光颜色 × cos(θ)) / 采样数
    
结果：辐照度贴图（低分辨率，因为漫反射很柔和）
  ┌────────────┐
  │  柔和的颜色  │
  │  渐变过渡    │
  └────────────┘
```

#### 使用

```glsl
// 在 Shader 中，用法线方向采样辐照度贴图
vec3 irradiance = texture(irradianceMap, fragNormal).rgb;
vec3 diffuse = irradiance * albedo / PI;
```

### 6.4 镜面反射 IBL：预滤波环境贴图 + BRDF 积分

#### 思路

镜面反射的积分比漫反射复杂得多，因为它还依赖**粗糙度**和**观察方向**。

Split-Sum 近似（Epic Games 提出）把积分拆成两部分：

$$\int L_i \cdot f_r \cdot (\vec{N} \cdot \omega_i) \, d\omega_i \approx \underbrace{\left(\int L_i \, d\omega_i\right)}_{\text{预滤波环境贴图}} \times \underbrace{\left(\int f_r \, d\omega_i\right)}_{\text{BRDF LUT}}$$

#### 第一部分：预滤波环境贴图（Pre-filtered Environment Map）

对不同粗糙度，对环境贴图做不同级别的模糊：

```
roughness = 0.0：   roughness = 0.3：   roughness = 0.7：
  清晰的环境反射       中等模糊              高度模糊
  （像镜子）           （像塑料）             （像粗糙金属）
  
Mip Level 0          Mip Level 2-3         Mip Level 5-6
```

在 Shader 中，用粗糙度选择 Mip Level：

```glsl
vec3 R = reflect(-V, N);
float lod = roughness * MAX_MIP_LEVEL;  // MAX_MIP_LEVEL 通常是 5~7
vec3 prefilteredColor = textureLod(prefilterMap, R, lod).rgb;
```

#### 第二部分：BRDF 积分贴图（BRDF LUT）

一个 2D 查找表，输入 $(\vec{N} \cdot \vec{V}$, roughness$)$，输出两个系数（scale 和 bias）：

```glsl
vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;

vec3 specularIBL = prefilteredColor * (F0 * brdf.x + brdf.y);
```

### 6.5 完整的 IBL 光照

```glsl
// 漫反射环境光
vec3 irradiance = texture(irradianceMap, N).rgb;
vec3 diffuse = irradiance * albedo * ao / PI;

// 镜面反射环境光
vec3 R = reflect(-V, N);
float lod = roughness * MAX_MIP_LEVEL;
vec3 prefilteredColor = textureLod(prefilterMap, R, lod).rgb;
vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

// 总环境光
vec3 ambient = diffuse + specular;
```

### 6.6 在引擎中的典型流程

```
离线预计算（或在编辑器中触发）：
  1. 加载 HDR 环境贴图
  2. 生成辐照度贴图（卷积模糊）
  3. 生成预滤波环境贴图（多级模糊）
  4. 生成 BRDF LUT（2D 查找表）
  5. 保存为纹理文件

运行时：
  1. 加载三张预计算纹理
  2. 在 Shader 中采样
  3. 和直接光照叠加
```

---

## 第七章 卡通渲染：二次元的光照哲学

### 7.1 卡通渲染的核心思想

PBR 追求物理真实，卡通渲染追求**艺术表达**。二次元的光影不是"真实的光照"，而是"好看的光照"。

**核心区别**：

| 特征 | PBR（写实） | 卡通渲染 |
|------|-------------|----------|
| 明暗过渡 | 平滑渐变 | 硬边分级（2~3 级） |
| 高光 | 物理正确的高光瓣 | 形状可控（圆形、条状） |
| 阴影颜色 | 自动变暗 | 可以指定阴影色（偏冷/偏暖） |
| 菲涅尔 | 物理正确 | 用于轮廓线或特殊效果 |
| 目标 | 像照片 | 像动画截图 |

### 7.2 分级光照（Cel Shading / Toon Shading）

#### 核心：把连续的光照离散化

```
PBR 的光照：              卡通渲染的光照：
  1.0 ┤                    1.0 ┤━━━━━━━┓
      │  ╱                 0.5 ┤        ┃━━━━━━┓
  0.5 ┤╱                    0.2 ┤                 ┗━━━━
      │╱                     0.0 ┤
  0.0 ┤━━━━                    ───┴──────────────────
      └──── NdotL                  NdotL
      连续渐变                     2~3 级硬切
```

#### 实现方法一：阶跃函数

```glsl
float NdotL = dot(N, L);

// 最简单的分级：亮 / 暗 两级
float toon;
if (NdotL > 0.5) toon = 1.0;       // 亮面
else if (NdotL > 0.0) toon = 0.5;  // 中间调
else toon = 0.2;                     // 暗面

vec3 color = albedo * lightColor * toon;
```

#### 实现方法二：Ramp Texture（渐变贴图）—— 更灵活

```glsl
float NdotL = dot(N, L);

// 把 NdotL 从 [-1, 1] 映射到 [0, 1]
float ramp = NdotL * 0.5 + 0.5;

// 用 ramp 采样一张渐变贴图
vec3 toonColor = texture(toonRamp, vec2(ramp, 0.5)).rgb;

vec3 color = albedo * lightColor * toonColor;
```

```
渐变贴图（Ramp Texture）示例：

  2 级：           3 级：              多级（柔和卡通）：
  ┌──┬──────┐     ┌──┬──┬──────┐     ┌────────────────┐
  │▒▒│██████│     │▒▒│▓▓│██████│     │▒▒▓▓▓██▇▇▆▅▅▄▄│
  └──┴──────┘     └──┴──┴──────┘     └────────────────┘
  0  0.3  1.0     0  0.2 0.5  1.0    硬边但过渡稍柔和
```

### 7.3 阴影颜色控制

在二次元中，阴影不是简单地变暗，而是**偏色**：

```
亮面：暖色调（偏黄/橙）
阴影：冷色调（偏紫/蓝）

  亮面：(1.0, 0.9, 0.8)
  阴影：(0.6, 0.5, 0.7)  ← 偏紫
```

```glsl
uniform vec3 shadowColor = vec3(0.6, 0.5, 0.7);  // 阴影色

float toon = getToonLevel(NdotL);  // 0.2, 0.5, 或 1.0

// 亮面用光源色，阴影面用阴影色
vec3 lightContrib = mix(shadowColor, lightColor, toon);
vec3 color = albedo * lightContrib;
```

### 7.4 轮廓线（Outline）

二次元角色的黑色描边是标志性特征。

#### 方法一：背面法线膨胀法（最常用）

```glsl
// ===== 轮廓线 Pass =====
// 顶点着色器
void main() {
    // 沿法线方向膨胀
    vec3 pos = position + normal * outlineWidth;
    gl_Position = vp * model * vec4(pos, 1.0);
}

// 片元着色器
void main() {
    outColor = vec4(outlineColor, 1.0);  // 通常是黑色
}

// 渲染设置：
// - 剔除正面（VK_CULL_MODE_FRONT_BIT），只画背面
// - 在正常渲染之前画（作为底层）
```

```
正常模型：               膨胀后的背面：
  ┌─────┐                ┌───────────┐
  │     │                │ ┌─────┐   │
  │     │                │ │     │   │ ← 多出来的边框
  │     │                │ │     │   │   就是轮廓线
  └─────┘                │ └─────┘   │
                         └───────────┘
```

#### 轮廓线宽度控制

```glsl
// 根据距离调整宽度（远处粗，近处细）
float dist = length(viewPos - fragPos);
float width = outlineWidth * dist * 10.0;

// 根据法线方向调整（边缘粗，正面细）
float edgeFactor = 1.0 - abs(dot(N, V));
width *= edgeFactor;
```

### 7.5 头发高光（天使环 / Angel Ring）

二次元角色的头发上通常有一道弧形高光：

```
      ╭──────────╮
     │  ╭──────╮  │
     │  │ 高光  │  │  ← 天使环
      ╰─╯      ╰─╯
       头发
```

#### 实现思路

```glsl
// 方法一：用一张特殊 UV 的高光贴图
vec3 hairSpecular = texture(hairSpecularMap, fragUV).rgb;

// 方法二：用各向异性高光（Kajiya-Kay 模型简化版）
vec3 hairTangent = normalize(cross(N, vec3(0, 1, 0)));  // 头发方向
float TdotL = dot(hairTangent, L);
float TdotV = dot(hairTangent, V);
float hairSpec = sqrt(1.0 - TdotL * TdotL) * sqrt(1.0 - TdotV * TdotV)
               + TdotL * TdotV;
hairSpec = pow(max(hairSpec, 0.0), hairShininess);
```

### 7.6 完整的二次元 Shader 框架

```glsl
void main() {
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(viewPos - fragPos);
    vec3 L = normalize(lightPos - fragPos);
    float NdotL = dot(N, L);
    
    // 1. 分级光照
    float ramp = NdotL * 0.5 + 0.5;
    vec3 toonLight = texture(toonRamp, vec2(ramp, 0.5)).rgb;
    
    // 2. 阴影颜色
    vec3 shadowCol = shadowColor;
    vec3 lightContrib = mix(shadowCol, lightColor, toonLight);
    
    // 3. 基础颜色
    vec3 baseColor = texture(albedoMap, fragUV).rgb;
    vec3 color = baseColor * lightContrib;
    
    // 4. 菲涅尔边缘光（Rim Light）
    float rim = 1.0 - max(dot(N, V), 0.0);
    rim = smoothstep(0.6, 1.0, rim);  // 只在边缘出现
    color += rimColor * rim * rimIntensity;
    
    // 5. 特殊高光（如头发天使环）
    // color += hairSpecular;
    
    // 6. Gamma
    color = pow(color, vec3(1.0 / 2.2));
    
    outColor = vec4(color, 1.0);
}
```

### 7.7 二次元渲染的典型参数

| 参数 | 典型值 | 说明 |
|------|--------|------|
| 分级数 | 2~3 级 | 亮面、中间调、暗面 |
| 阴影色 | 偏紫/蓝 | `(0.6, 0.5, 0.7)` 左右 |
| 轮廓线宽度 | 0.5~2.0 像素 | 根据角色大小调整 |
| 轮廓线颜色 | 深棕/深紫 | 不一定是纯黑 |
| 边缘光强度 | 0.3~0.8 | 不要太亮，点缀即可 |
| Ramp 过渡 | 硬边为主 | 可以有一点点过渡 |

### 7.8 主流二次元引擎方案参考

| 游戏 | 方案特点 |
|------|----------|
| 原神 (Unity) | 分级光照 + Ramp 贴图 + SDF 面部阴影 + 后处理描边 |
| 崩坏：星穹铁道 | 在原神基础上增加更精细的 SSS 近似 |
| 罪恶装备 (UE) | 2.5D 风格，手绘法线 + 自定义光照模型 |
| 蔚蓝档案 (Unity) | 简洁的 2~3 级 shading，强调角色轮廓 |

---

## 附录 光照参数速查表

### 常见材质的 PBR 参数

| 材质 | Base Color | Metallic | Roughness | F0 |
|------|-----------|----------|-----------|-----|
| 皮肤 | (0.6, 0.45, 0.35) | 0 | 0.5~0.7 | 0.028 |
| 白色塑料 | (0.8, 0.8, 0.8) | 0 | 0.4~0.6 | 0.04 |
| 红色油漆 | (0.6, 0.05, 0.05) | 0 | 0.2~0.3 | 0.04 |
| 湿润沥青 | (0.1, 0.1, 0.1) | 0 | 0.3~0.5 | 0.04 |
| 铁 | (0.56, 0.57, 0.58) | 1 | 0.3~0.6 | F0=albedo |
| 金 | (1.0, 0.71, 0.29) | 1 | 0.1~0.3 | F0=albedo |
| 铜 | (0.95, 0.64, 0.54) | 1 | 0.2~0.5 | F0=albedo |
| 铝 | (0.91, 0.92, 0.92) | 1 | 0.1~0.3 | F0=albedo |
| 水 | — | 0 | 0.0~0.1 | 0.02 |
| 玻璃 | — | 0 | 0.0~0.05 | 0.04 |

### 光源参数参考

| 光源类型 | 颜色 | 强度 | 说明 |
|----------|------|------|------|
| 太阳光 | (1.0, 0.95, 0.85) | 10~30 klux | 暖白色 |
| 阴天天空 | (0.7, 0.8, 1.0) | 2~10 klux | 偏蓝 |
| 室内灯光 | (1.0, 0.9, 0.7) | 100~500 lux | 暖色 |
| 月光 | (0.6, 0.7, 0.9) | 0.1~0.3 lux | 冷蓝色 |
| 蜡烛 | (1.0, 0.7, 0.3) | 很近时才有效 | 很暖 |

### 公式速查

| 名称 | 公式 |
|------|------|
| Lambert 漫反射 | $L_d = k_d (\vec{N} \cdot \vec{L})$ |
| Blinn-Phong 高光 | $L_s = k_s (\vec{N} \cdot \vec{H})^n$ |
| 半程向量 | $\vec{H} = \text{normalize}(\vec{L} + \vec{V})$ |
| 反射向量 | $\vec{R} = 2(\vec{N} \cdot \vec{L})\vec{N} - \vec{L}$ |
| Schlick 菲涅尔 | $F = F_0 + (1-F_0)(1-\cos\theta)^5$ |
| GGX NDF | $D = \frac{\alpha^2}{\pi((\vec{N}\cdot\vec{H})^2(\alpha^2-1)+1)^2}$ |
| Cook-Torrance | $f_s = \frac{DFG}{4(\vec{N}\cdot\vec{V})(\vec{N}\cdot\vec{L})}$ |
| 能量守恒 | $k_d = (1 - k_s)(1 - \text{metallic})$ |

---

> **最后的话**
>
> 光照和材质是渲染中最"看得见"的部分。从 Lambert 到 PBR 到卡通渲染，核心思路是一样的：**理解光怎么和表面交互，然后用数学去模拟它。**
>
> PBR 的价值在于给你一套**统一的框架**——金属度、粗糙度、基础色这三个参数就能描述几乎所有常见材质。而卡通渲染的价值在于告诉你——**物理正确不是唯一的目标，好看才是。**
>
> 现在，打开你的 Shader，从 Blinn-Phong 开始写起，一步步升级到 PBR，最后尝试卡通渲染。每写一步，看看画面发生了什么变化。这种"写代码 → 看效果 → 理解原理"的循环，是最好的学习方式。
