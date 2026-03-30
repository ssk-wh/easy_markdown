# Nested Structures Test

## List within Quote

> This is a quote containing a list:
> 1. Item one
> 2. Item two
>    - Sub-item
>    - Another sub
> 3. Item three

## Quote within List

1. Regular list item
2. List item with quote:
   > This is a quote within a list item
   >
   > It can span multiple lines
3. Another list item

## Code within Quote

> Example code within quote:
>
> ```python
> def process():
>     return True
> ```
>
> This is regular text after the code block

## Table within Quote

> | Header 1 | Header 2 |
> |----------|----------|
> | Value 1  | Value 2  |
> | Value 3  | Value 4  |

## Deeply Nested Structure

1. Level 1
   - Level 2 item 1
     > Quote at level 3
     >
     > With multiple lines
     >
     > ```code
     > and code
     > ```
   - Level 2 item 2
     1. Ordered sub-item
     2. Another ordered
   - Level 2 item 3
     * Unordered sub-item
     * Another unordered
2. Level 1 item 2
   - Level 2
     - Level 3
       - Level 4
         - Level 5 item 1
         - Level 5 item 2

## Mixed Inline Formatting

This paragraph contains:
- **Bold with `code` inside**
- *Italic with `code` inside*
- ***Bold and italic with `code`***
- `Code with **bold** inside` (if supported)

## List with Complex Items

1. Item with table:
   | Col1 | Col2 |
   |------|------|
   | A    | B    |

2. Item with code:
   ```
   code block
   ```

3. Item with quote:
   > A quote
