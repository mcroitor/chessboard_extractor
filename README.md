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
# blur size
blur_size        = 17

blur_standard   = 0
blur_median     = 1
blur_gaussian   = 1

contour_delta    = 20

# page file format
format   = png

# delta for comparing in pixels
gap      = 50

#input directory
input    = .\in\

# max board size
max_board_size   = 900

# min board size
min_board_size   = 600

#output directory
output   = .\out\

remove_non_squares       = 1

# the red contour thickness, drawed over 
thickness        = 5

# Canny algorithm treshold
treshold         = 100
```

This config specifies limits for board sizes, input and output directories, page file extention for analyzing.

Main motif of file format introducing is possibility to write to the input directory different formats.

## compile

Project use OpenCV library for contours detection. 

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

You can play with `blur_size`  and `treshold` values for detection improvement. Also you can enable / disable
bluring altorithms.
