
# definitions
objRRC = bylabel.o
srcRRC = bylabel.cpp

#linker to use
lnk = g++
#compiler to use
cc = gcc -std=c++11
#uncomment for debugging
dbg = -g -Wall

# MAKE it happen
all: bylabel

bylabel: $(objRRC)
	$(lnk) $(dbg) -o bylabel $(objRRC) -L lib -Wl,-rpath,'$$ORIGIN/lib' `pkg-config --libs opencv` EDLib.a EDLinesLib.a

$(objRRC): $(srcRRC)
	$(cc) $(dbg) `pkg-config --cflags opencv` -c $(srcRRC)

clean:
	@rm -f $(objRRC) bylabel
