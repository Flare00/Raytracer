#permet la compilation de .exe windows, n�cessite d'installer g++-mingw-w64
CXXW64 = x86_64-w64-mingw32-g++-posix
CXX = g++

#Dossier du code source et des header.
SRCFOLDER = src
HEADERFOLDER = src
HEADERFOLDERW64 = 

#FLAGS 
#CXXFLAGS = -iquote $(HEADERFOLDER) -g
CXXFLAGS = -Ofast -lm -g


EXENAME = main
#GET SRCs
SRC = $(wildcard $(SRCFOLDER)/*.cpp) 
MAIN = main.cpp
LIBS = -lGL -lGLU -lglut -pthread
LIBSW64 = -I"./W64/include" -lopengl32 -lglu32 ./W64/lib/libfreeglut.a -lpthread #-static-libgcc -static-libstdc++ -g -static-libgcc -static-libstdc++
#Automatic
OBJ = $(SRC:.cpp=.o)
OBJW64 = $(SRC:.cpp=.obj)

all : linux

linux : $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(EXENAME) $(MAIN) $(OBJ) $(LIBS)
	@echo "Compilation de l'executable linux : $(EXENAME)"
	
w64 : $(OBJW64)
	$(CXXW64) $(CXXFLAGS) -o $(EXENAME).exe $(MAIN) $(OBJW64) $(LIBSW64)
	@echo "Compilation de l'executable Win64 : $(EXENAME).exe"
	
%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ 
%.obj : %.cpp
	$(CXXW64) -c $< -o $@ $(LIBSW64)

clean:
	rm -f src/*.o
	rm -f *.o
	rm -f src/*.obj
	rm -f *.obj
	@echo "*.o et *.obj effac�s !"