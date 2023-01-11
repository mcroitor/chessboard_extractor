PREFIX =
OBJDIR = obj/$(PREFIX)
BUILDDIR = build/$(PREFIX)

CC=g++
LINKER=g++

CXXFLAGS = `pkg-config --cflags opencv4`
LDFLAGS = `pkg-config --libs opencv4`

CXXFILES=functions.cpp properties.cpp board_detector.cpp extractboards.cpp markboards.cpp
OBJFILES=$(OBJDIR)/functions.o \
	$(OBJDIR)/properties.o \
	$(OBJDIR)/board_detector.o
APP=extractboards
APP2=markboards


.phony: all

all: prepare build

clean:
	rm -R $(OBJDIR)
	rm -R $(BUILDDIR)

prepare:
	@echo "create structure"
	mkdir -p $(OBJDIR)
	mkdir -p $(BUILDDIR)

build: $(APP) $(APP2)	

$(APP): $(OBJFILES) $(OBJDIR)/extractboards.o
	@echo "build $(BUILDDIR)/$(APP)"
	$(LINKER) -o $(BUILDDIR)/$(APP) $(OBJFILES) $(OBJDIR)/extractboards.o $(LDFLAGS)

$(APP2): $(OBJFILES) $(OBJDIR)/markboards.o
	@echo "build $(BUILDDIR)/$(APP2)"
	$(LINKER) -o $(BUILDDIR)/$(APP2) $(OBJFILES) $(OBJDIR)/markboards.o $(LDFLAGS)

$(OBJDIR)/functions.o : functions.cpp
	$(CC) $(CXXFLAGS) -c -o $(OBJDIR)/functions.o functions.cpp

$(OBJDIR)/properties.o : properties.cpp
	$(CC) $(CXXFLAGS) -c -o $(OBJDIR)/properties.o properties.cpp

$(OBJDIR)/board_detector.o : board_detector.cpp
	$(CC) $(CXXFLAGS) -c -o $(OBJDIR)/board_detector.o board_detector.cpp

$(OBJDIR)/extractboards.o : extractboards.cpp
	$(CC) $(CXXFLAGS) -c -o $(OBJDIR)/extractboards.o extractboards.cpp

$(OBJDIR)/markboards.o : markboards.cpp
	$(CC) $(CXXFLAGS) -c -o $(OBJDIR)/markboards.o markboards.cpp
