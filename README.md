# chessboard_extractor

ChessBoard extractor is a small tool that helps extract chess diagrams from scanned pages.

Program can detect any square (from defined size, default 200), it can creates misdetection.

## usage

Command line usage:

```cmd
extractboards --intput=<input_dir> --output=<output_dir> --format=<page_extension>
```

Or you can create `config.ini` file inplace:

```ini
blur_size        = 11
contour_delta    = 5
format   = png
gap      = 20
input    = .\
max_board_size   = 800
min_board_size   = 200
output   = .\out\
remove_nested    = 1
remove_non_squares       = 1
remove_same      = 1
treshold         = 100
```

## compile

project use OpenCV library for contours detection. My `Makefile` is strictly linked with my
PC environment. Feel free to edit it.

If you have `MSYS` with `Mingw64` environment, install OpenCV ( `pacman -Ss opencv` will help you).

```bash
git clone git@github.com:mcroitor/chessboard_extractor.git
cd chessboard_extractor
make
```

## recomendations

This tool is not a panacea: sometimes it brokes detection if diagram margins contains gap.

Good idea will be page preparation, with ScanKromsator, for example. This preparation will make
white pages white and black lines - black.

As recommendation, you can check initially chess board size and set dimmension using `min_board_size`
and `max_board_size` options.
