# chessboard_extractor

`chessboard_extractor` is a small utility for extracting chess diagrams from scanned pages.

The detector searches for large, near-square regions that are likely to be chess diagrams. Depending on scan quality and parameter values, false positives and misses are possible.

## Usage

Extract diagrams:

```cmd
extractboards --input=<input_dir> --output=<output_dir> --format=<page_extension>
```

Mark detected diagram bounds on page images:

```cmd
markboards --input=<input_dir> --output=<output_dir> --format=<page_extension>
```

You can also configure parameters in `config.ini`:

```ini
# blur kernel size
blur_size        = 21

blur_standard   = 0
blur_median     = 0
blur_gaussian   = 1

# close small edge gaps after Canny
close_gaps      = 1
morph_kernel_size = 5
morph_iterations  = 1

contour_delta    = 50

# input page file extension
format   = png

# tolerance for approximate comparisons (pixels)
gap      = 50

# merge nearby rectangles during optimize step (pixels)
merge_gap = 8

# input directory
input    = .\in\

# max board size
max_board_size   = 700

# min board size
min_board_size   = 500

# output directory
output   = .\out\

remove_non_squares       = 1

# red rectangle thickness for markboards output
thickness        = 5

# Canny threshold (legacy option name kept for compatibility)
treshold         = 100
```

## Parameters and tuning effects

The table below summarizes expected behavior when increasing or decreasing each parameter.

| Parameter | Increase / enable | Decrease / disable |
|---|---|---|
| `blur_size` | More smoothing, less noise, but thin edges may disappear | More detail preserved, but more noisy contours |
| `blur_gaussian` | Soft denoise, usually a good default for scans | More raw/noisy edges |
| `blur_median` | Better for salt-and-pepper noise and print defects | Less robust to isolated defects |
| `blur_standard` | Strong averaging; can suppress weak edges | Preserves local details better |
| `treshold` | Stricter Canny; fewer edges and contours | More sensitive Canny; more edges and false contours |
| `contour_delta` | Stronger contour simplification, rougher shapes | More accurate shape, but noisier contours |
| `min_board_size` | Rejects more small candidates; fewer false positives | Keeps smaller candidates; potentially more false positives |
| `max_board_size` | Allows larger candidates | Rejects larger candidates and merged areas |
| `remove_non_squares` | Keeps only near-square candidates | Keeps all shapes (useful for diagnostics) |
| `gap` | More tolerant near-square check | Stricter square check |
| `close_gaps` | Enables morphological closing; helps broken diagram borders | Keeps raw broken edges; may miss damaged borders |
| `morph_kernel_size` | Closes larger gaps; may over-merge nearby edges | Gentler closing; less over-merge risk |
| `morph_iterations` | Stronger repeated closing effect | Weaker closing effect |
| `merge_gap` | Merges nearby rectangles without direct overlap | Merges only very close/intersecting rectangles |
| `thickness` (`markboards`) | Thicker red debug rectangles | Thinner debug rectangles |

## Practical presets

Default preset for low-quality scans with broken diagram borders:

```ini
blur_size = 21
blur_gaussian = 1
blur_median = 0
blur_standard = 0

close_gaps = 1
morph_kernel_size = 5
morph_iterations = 1
merge_gap = 8
```

If border breaks are stronger:

- raise `morph_kernel_size` to `7`
- optionally raise `morph_iterations` to `2`
- raise `merge_gap` to `10..12`

If false merges appear:

- lower `merge_gap` first
- then lower `morph_kernel_size` or `morph_iterations`

## Troubleshooting by symptoms

### Too many missed diagrams

- lower `treshold` (for example `100 -> 80`)
- increase `close_gaps` robustness: `morph_kernel_size 5 -> 7`
- increase `merge_gap` (`8 -> 10`)
- temporarily set `remove_non_squares = 0` to inspect candidates

### Too many false positives

- increase `treshold` (`100 -> 120`)
- increase `min_board_size`
- decrease `max_board_size`
- keep `remove_non_squares = 1`
- reduce `merge_gap` (`10 -> 6`)

### Broken diagram border is not detected

- set `close_gaps = 1`
- increase `morph_kernel_size` (`5 -> 7`)
- if needed, increase `morph_iterations` (`1 -> 2`)
- if candidate is split into multiple parts, increase `merge_gap`

### Neighboring objects are merged into one large box

- lower `merge_gap`
- lower `morph_kernel_size`
- lower `morph_iterations`

### Detected box is not square enough

- lower `gap` for stricter square check
- keep `remove_non_squares = 1`

### Useful debugging workflow

- run `markboards` first to inspect red boxes
- tune one parameter at a time
- after any change, compare page-by-page counts

## Build

The project uses OpenCV for contour detection.

If you use MSYS2/Mingw64, install OpenCV (for example, `pacman -Ss opencv`).

```bash
git clone git@github.com:mcroitor/chessboard_extractor.git
cd chessboard_extractor
make
```

## Recommendations

This tool is not a silver bullet: detection quality can drop on low-quality scans and broken diagram borders.

Pre-processing scans (background normalization, contrast improvement, line enhancement) usually improves results.

It is also recommended to estimate your real board size first and set `min_board_size` / `max_board_size` as tightly as possible for your dataset.
