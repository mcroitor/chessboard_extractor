OBJDIR = obj
BUILDDIR = build

TOOLSET=/mingw64

CC=$(TOOLSET)/bin/g++
LINKER=$(TOOLSET)/bin/g++
CXXFLAGS=-I$(TOOLSET)/include/opencv4
LDFLAGS=-L$(TOOLSET)/lib -lopencv_core.dll -lopencv_imgproc.dll \
  -lopencv_flann.dll -lopencv_imgcodecs.dll \
  -lopencv_videoio.dll -lopencv_highgui.dll \
  -lopencv_ml.dll -lopencv_video.dll \
  -lopencv_objdetect.dll -lopencv_photo.dll \
  -lopencv_features2d.dll -lopencv_calib3d.dll \
  -lopencv_stitching.dll
# CXXFLAGS = `pkg-config --cflags opencv`
# LDFLAGS = `pkg-config --libs opencv`

CXXFILES=functions.cpp properties.cpp board_detector.cpp main.cpp
OBJFILES=$(OBJDIR)/functions.o $(OBJDIR)/properties.o $(OBJDIR)/board_detector.o $(OBJDIR)/main.o
APP=extractboards


.phony: all

all: prepare build

clean:
	rm -R $(OBJDIR)
	rm -R $(BUILDDIR)

prepare:
	@echo "create structure"
	mkdir -p $(OBJDIR)
	mkdir -p $(BUILDDIR)

build: $(OBJFILES)
	@echo "build $(BUILDDIR)/$(APP)"
	$(LINKER) -o $(BUILDDIR)/$(APP) $(OBJFILES) $(LDFLAGS)

$(OBJDIR)/functions.o : functions.cpp
	$(CC) $(CXXFLAGS) -c -o $(OBJDIR)/functions.o functions.cpp

$(OBJDIR)/properties.o : properties.cpp
	$(CC) $(CXXFLAGS) -c -o $(OBJDIR)/properties.o properties.cpp

$(OBJDIR)/board_detector.o : board_detector.cpp
	$(CC) $(CXXFLAGS) -c -o $(OBJDIR)/board_detector.o board_detector.cpp

$(OBJDIR)/main.o : main.cpp
	$(CC) $(CXXFLAGS) -c -o $(OBJDIR)/main.o  main.cpp
