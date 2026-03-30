# Comprehensive Markdown Test Document

This document contains all common markdown formatting elements for testing rendering capabilities.

## 基本格式测试 / Basic Formatting

### 加粗文本和斜体 (Bold and Italic)

这是一个**加粗的文本**，这是一个*斜体的文本*。

You can also use **bold text** and *italic text* in English.

还可以***混合加粗和斜体***的内容。

Mixed formatting: This is **bold with *italic inside*** the bold text.

---

## 标题测试 (Headings)

### H3 - 三级标题

#### H4 - 四级标题

##### H5 - 五级标题

###### H6 - 六级标题

---

## 行内代码和代码块 (Inline Code and Code Blocks)

Here is some `inline code` in a sentence. And `another piece of code`.

Chinese content with `行内代码` example.

### C++ 代码示例

```cpp
#include <iostream>
#include <string>

// 计算两个数的和
int add(int a, int b) {
    return a + b;
}

int main() {
    std::string message = "Hello, 世界!";
    int result = add(5, 3);
    std::cout << message << " Result: " << result << std::endl;
    return 0;
}
```

### Python 代码示例

```python
def fibonacci(n):
    """生成斐波那契数列"""
    a, b = 0, 1
    for _ in range(n):
        yield a
        a, b = b, a + b

# 使用示例
for num in fibonacci(10):
    print(f"Fibonacci: {num}")
```

### JavaScript 代码示例

```javascript
// 异步获取数据
async function fetchUserData(userId) {
    const response = await fetch(`/api/users/${userId}`);
    const data = await response.json();

    return {
        name: data.name,
        email: data.email,
        // 中文注释
        status: "活跃" // Active
    };
}

// 链式调用
const users = [1, 2, 3]
    .map(id => fetchUserData(id))
    .filter(user => user.status === "活跃");
```

### HTML/XML 代码示例

```html
<!DOCTYPE html>
<html>
<head>
    <title>测试页面</title>
    <meta charset="UTF-8">
</head>
<body>
    <h1>欢迎来到我的页面</h1>
    <p>这是一个 <strong>测试</strong> 页面。</p>
    <!-- XML 注释 -->
    <config>
        <name>SimpleMarkdown</name>
        <version>1.0.0</version>
    </config>
</body>
</html>
```

---

## 列表测试 (Lists)

### 无序列表 (Unordered List)

- 第一个顶级项目
  - 嵌套的第二级项目
    - 嵌套的第三级项目
      - 第四级项目
  - 同级的另一个第二级项目
- 第二个顶级项目
  - 它的子项目
- 第三个顶级项目

* 使用星号标记的列表
  * 嵌套项目
    * 更深层级
+ 使用加号标记的列表
  + 混合标记也可以
  + 效果相同

### 有序列表 (Ordered List)

1. 第一项
2. 第二项
   1. 嵌套的第一个子项
   2. 嵌套的第二个子项
      1. 更深层级的项目
      2. 继续嵌套
   3. 回到第二级
3. 第三项
4. 第四项
   1. 子项 A
   2. 子项 B

### 混合列表 (Mixed Lists)

1. 顶级有序项目 1
   - 无序子项 A
   - 无序子项 B
     1. 嵌套的有序项
     2. 继续有序项
   - 无序子项 C
2. 顶级有序项目 2
   - 子项

---

## 表格测试 (Tables)

### 简单表格

| 列 1 | 列 2 | 列 3 |
|------|------|------|
| 数据 1 | 数据 2 | 数据 3 |
| A | B | C |
| 中文内容 | English Content | 混合 Mixed |

### 复杂表格（多行多列）

| 功能名称 | 描述 | 支持平台 | 状态 |
|---------|------|---------|------|
| 代码高亮 | 支持 20+ 编程语言 | Windows, macOS, Linux | ✓ 完成 |
| 实时预览 | 编辑时即时显示 | 全平台 | ✓ 完成 |
| 导出 PDF | 支持自定义样式 | Windows, macOS | ⚠️ 测试中 |
| 协作编辑 | 实时同步 | 全平台 | ❌ 规划中 |
| 插件系统 | 扩展自定义功能 | Windows, Linux | ✓ 完成 |

### 对齐表格

| 左对齐 | 居中 | 右对齐 |
|:-------|:-----:|--------:|
| 文本 | 中间 | 右边 |
| Apple | Banana | Cherry |
| 苹果 | 香蕉 | 樱桃 |

---

## 块引用测试 (Blockquotes)

> 这是一个基本的块引用。
> 第二行内容。

> 这是一个更长的块引用
>
> 它包含多个段落
>
> 和多行内容

> 嵌套引用示例
>
> > 这是第一级嵌套引用
> >
> > > 这是第二级嵌套引用
> > > 包含多行文本

> **加粗的引用内容**
>
> 包含 `代码` 的引用
>
> - 引用内的列表
> - 第二项

---

## 删除线测试 (Strikethrough)

这是~~删除线~~的文本。

~~整个句子都被删除~~ 这是正常文本。

混合: 这是**加粗的~~删除线~~内容**。

---

## 链接和图片 (Links and Images)

### 超链接

[Google](https://google.com)

[GitHub 项目](https://github.com)

[访问百度](https://baidu.com "百度搜索")

[带标题的链接示例](https://example.com "Example Domain")

### 参考链接

[Reference Link][ref1]

[Another Reference][ref2]

[ref1]: https://example.com
[ref2]: https://github.com

### 图片（使用占位符）

![替代文本](https://via.placeholder.com/400x300 "图片标题")

![代码示例缩略图](https://via.placeholder.com/200x150)

---

## 分隔线测试 (Horizontal Rules)

---

***

___

---

## 高级内容混合 (Advanced Mixed Content)

### 列表中包含代码块和引用

1. **安装步骤**

   首先，运行以下命令：

   ```bash
   git clone https://github.com/example/project.git
   cd project
   npm install
   ```

2. **配置部分**

   > 注意：配置文件必须是有效的 JSON 格式

   ```json
   {
     "version": "1.0.0",
     "name": "项目名称",
     "settings": {
       "theme": "dark",
       "language": "zh-CN"
     }
   }
   ```

3. **运行应用**

   ```bash
   npm start
   ```

### 引用中包含列表

> **重要的说明**
>
> 请按以下步骤进行操作：
>
> 1. 阅读文档
>    - 查看 README
>    - 查看 CONTRIBUTING 指南
> 2. 设置开发环境
>    - 安装依赖
>    - 配置编辑器
> 3. 开始贡献代码

### 表格与代码混合

| 语言 | Hello World 代码 | 复杂度 |
|------|------------------|--------|
| Python | `print("Hello, World!")` | 低 |
| Java | `System.out.println("Hello, World!");` | 中 |
| C++ | `std::cout << "Hello, World!";` | 中高 |

---

## 特殊字符测试 (Special Characters)

中文字符：你好，世界！中文测试内容。

特殊符号：@ # $ % ^ & * ( ) _ + - = [ ] { } ; : ' " < > , . ? /

数学符号：+ - × ÷ = ≠ < > ≤ ≥ ≈ ∞ √ ∑ ∏

货币符号：$ € ¥ £ ₹ ¢

箭头：→ ← ↑ ↓ ↔ ⇒ ⇐ ⇔

---

## 复杂嵌套场景 (Complex Nesting Scenarios)

- **复杂项目 1**

  这是项目中的描述文本。

  > 引用内容
  >
  > - 引用中的列表
  > - 多个项

  ```python
  # 代码示例
  def hello():
      return "世界"
  ```

  继续文本描述。

- **复杂项目 2**

  | 字段 | 值 |
  |------|-----|
  | 名称 | 值1 |
  | 数值 | 值2 |

  最后一段说明文本。

---

## 边界情况测试 (Edge Cases)

### 连续空行测试

这里有内容。


这是分隔后的内容。

### 长行测试

这是一个非常长的行，它包含了大量的文本内容以测试文本换行的行为。Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. 这里混合了中文和英文长文本。

### 空代码块

```

```

### 仅空白代码块

```

```

### 多种换行符

行 1
行 2

行 3

行 4

---

## 最终总结 (Summary)

本文档展示了 SimpleMarkdown 支持的所有主要 Markdown 格式：

- ✓ 文本格式 (加粗、斜体)
- ✓ 标题 (H1-H6)
- ✓ 代码块 (多语言)
- ✓ 行内代码
- ✓ 列表 (有序、无序、嵌套)
- ✓ 表格
- ✓ 块引用
- ✓ 链接
- ✓ 删除线
- ✓ 分隔线
- ✓ 复杂混合内容

用于完整的功能测试和渲染验证。
