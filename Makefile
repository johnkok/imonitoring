HEADERS = ioko.hpp

OBJECTS = \
	ioko.o \
	inverterConnect.o \
	dataProcessing.o \
	cameraConnect.o \
	imageProsessing.o \
	modemConnect.o \
	hid.o \
	weatherStation.o \
	peopledetect.o

CXXFLAGS = -O3 -o prog -g3 -rdynamic -D_GNU_SOURCE \
	-Wno-write-strings -Wno-unused-result \
	-L/usr/local/lib -I/usr/include/opencv -L/lib 

LIBS = 	-lopencv_imgproc  -lplplotcxxd \
	-lpthread -lcurl \
	-lstdc++ \
	-lopencv_core -lopencv_contrib -lopencv_legacy \
	-lopencv_ml -lopencv_video -lopencv_highgui \
        -lopencv_calib3d -lopencv_flann -lopencv_objdetect -lopencv_features2d \
	-ludev -lm

# -lopencv_imgcodecs

default: ioko

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

ioko: $(OBJECTS)
	gcc $(OBJECTS) $(LIBS) $(CXXFLAGS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f ioko

