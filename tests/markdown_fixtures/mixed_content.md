# Mixed Content Test

## Introduction

This document tests rendering of mixed Markdown content with various formatting options.

## Section 1: Lists and Code

Here's a list with some code:

1. First item with `code snippet`
2. Second item with **bold** text
3. Third item with code block:

```python
def example():
    return True
```

## Section 2: Quotes and Lists

> This quote contains a list:
>
> - Item 1
> - Item 2
>   - Nested item
> - Item 3

## Section 3: Tables and Text Formatting

| Feature | Status | Details |
|---------|--------|---------|
| Lists | ✓ | Ordered and unordered |
| **Code** | ✓ | Inline and blocks |
| *Quotes* | ✓ | Single and nested |

## Section 4: Complex Nesting

1. Top level item
   - Nested list item
     > With a quote inside
     >
     > ```code
     > and code
     > ```
   - Another nested item
2. Another top level
   - Sub-item with **bold** and *italic*
   - Sub-item with `code`

## Section 5: Multiple Code Blocks

Here's one example:

```cpp
void render() {
    paint();
    update();
}
```

And another:

```javascript
function validate(data) {
    return data !== null;
}
```

## Conclusion

This section tests the conclusion with a quote:

> "The best code is no code at all." — Unknown

---

Final paragraph with **bold**, *italic*, and `code` elements.
