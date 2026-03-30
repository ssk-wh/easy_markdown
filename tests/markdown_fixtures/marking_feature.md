# Marking Feature Test

## Introduction to Marking

This document is designed to test the marking/highlighting feature in SimpleMarkdown.

### Key Features to Mark

You should be able to:
1. Select text in the preview area
2. Right-click and choose "Mark"
3. See the text highlighted in yellow
4. View the marked section's heading highlighted in TOC
5. Clear all marks with the context menu

## Content Samples for Marking

### Code Block Section

Here's a code block that you can mark parts of:

```cpp
void RenderingValidator::validateLayout() {
    QVector<PixelRegion> regions = extractRegions();

    // This section tests alignment checking
    if (!validateAlignment(regions)) {
        logError("Alignment validation failed");
        return false;
    }

    // Mark the part about error handling
    logger.info("Validation complete");
}
```

### List Section

1. Important point to mark
2. Critical information here
3. Another point
   - Sub-point 1
   - Sub-point 2 (also important)
4. Final point

### Quote Section

> "Code is like humor. When you have to explain it, it's bad." — Cory House
>
> Try marking this quote when you read it.

### Table Section

| Feature | Importance | Marks? |
|---------|-----------|--------|
| Rendering | Critical | ✓ Mark this |
| DPI Support | High | ✓ Mark this |
| Selection | Medium | ✓ Mark this |
| Marking | High | ✓ Mark this |

## Testing Workflow

1. **Select Content**: Click and drag to select text
2. **Mark Content**: Right-click and choose "Mark"
3. **Verify TOC**: Check if heading is highlighted in TOC panel
4. **Clear Marks**: Right-click and choose "Clear all marks"
5. **Verify Clearing**: Check if all marks and TOC highlights are gone

## Sections to Try

### Section A: Mark This Section

This is content in Section A that you should try to mark. The entire paragraph or just a portion can be marked.

### Section B: Mark This Too

Multiple marks can be added to different sections. Each section's heading will be highlighted if any content in it is marked.

### Section C: Complex Content to Mark

1. Marked list items
2. Marked code:
   ```
   marked code content
   ```
3. Marked quotes:
   > This can be marked
4. Marked tables

## Summary

After testing all the marking features, you should:
- Have multiple sections with marks
- See all marked sections' headings highlighted in TOC
- Be able to clear all marks at once
- Understand how the marking feature works
