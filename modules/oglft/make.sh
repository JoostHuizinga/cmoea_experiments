g++ -c OGLFT.cpp -I/usr/local/Cellar/freetype/2.6_1/include/freetype2/ -I. -I/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers/ -DAPPLE -DOGLFT_NO_SOLID -DOGLFT_NO_QT
ar rcs liboglft.a OGLFT.o
