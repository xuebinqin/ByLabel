#all:
#	export LD_LIBRARY_PATH="."
#	g++ -o SCBT_Edge SalientClosedBoundaryTracking_Edge.cpp EDLib.a libopencv_imgproc.so.2.4.5 libopencv_core.so.2.4.5


#clean:
#	rm -rf SCBT_Edge core


# definitions
objRRC = GDTL.o
srcRRC = GDTL.cpp

#linker to use
lnk = g++
#compiler to use
cc = gcc -std=c++11
#uncomment for debugging
dbg = -g -Wall

# MAKE it happen

all: GDTL

GDTL: $(objRRC)
	$(lnk) $(dbg) -o GDTL $(objRRC) -L lib -Wl,-rpath,'$$ORIGIN/lib' `pkg-config --libs opencv` EDLib.a EDLinesLib.a

#GDTL: $(objRRC)
#	$(lnk) $(dbg) -o GDTL $(objRRC) EDLib.a EDLinesLib.a `pkg-config --libs opencv`

$(objRRC): $(srcRRC)
	$(cc) $(dbg) `pkg-config --cflags opencv` -c $(srcRRC)

clean:
	@rm -f $(objRRC) GDTL
