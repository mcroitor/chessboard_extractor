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
format          = tif
gap             = 50
input           = .\in\
min_board_size  = 200
output          = .\out\
treshold        = 100
```

## compile

project use OpenCV library for contours detection.

