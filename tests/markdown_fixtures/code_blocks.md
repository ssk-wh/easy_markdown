# Code Blocks Test

## C++ Code Block

```cpp
void initPlayer() {
    if (m_player)
        return;

    const auto extendMap = map.value("extend").toMap();
    m_player->setPlayerDocCtxId(ctxId);
    m_player->triggerAction("initPage");
}
```

## Python Code Block

```python
def calculate_metrics(font_size, dpi_ratio):
    logical_height = font_size * 1.4
    physical_height = logical_height * dpi_ratio
    return {
        "logical": logical_height,
        "physical": physical_height
    }
```

## JavaScript Code Block

```javascript
function validateRenderingConsistency(dpi1x, dpiOther, tolerance = 0.5) {
    const logical1x = dpi1x / 1.0;
    const logicalOther = dpiOther / dpiRatio;

    const diff = Math.abs(logical1x - logicalOther);
    return diff <= tolerance;
}
```

## Bash Code Block

```bash
#!/bin/bash
cd D:\iflytek_projects\simple_markdown
build_on_win.bat clean
build_on_win.bat release
```

## Very Long Line in Code Block

```
This is a very long line in a code block that should not cause any rendering issues or create unexpected whitespace. The line should be properly displayed and wrapped if necessary without breaking the layout of surrounding content.
```
