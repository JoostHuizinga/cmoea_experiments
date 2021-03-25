/*
 * pngwriter_util.hpp
 *
 *  Created on: Jul 6, 2018
 *      Author: joost
 */

#ifndef MODULES_PNGWRITER_PNGWRITER_UTIL_HPP_
#define MODULES_PNGWRITER_PNGWRITER_UTIL_HPP_
#include <boost/lexical_cast.hpp>

#include "pngwriter.h"

#ifdef USE_SDL
#include <SDL.h>
#endif


void saveFrame(unsigned char *image, int width, int height, int bytesPerPixel, std::string frame_name){
	pngwriter PNG(width, height, 1.0, frame_name.c_str());
	int nBytes = width * height * bytesPerPixel;
	size_t x = 1;   // start the top and leftmost point of the window
	size_t y = 1;
	double R, G, B, A;
	if(bytesPerPixel != 4){
		std::cerr << "Formats other than ARGB are currently not supported" << std::endl;
		return;
	}
	for(size_t i=0; i<nBytes; i++){
		switch(i%bytesPerPixel){
		case 3:{
			B = (double) image[i];
			PNG.plot(x, y, R/256, G/256, B/256);
			if( x == width ){
				x=1;
				y++;
			}
			else{
				x++;
			}
			break;
		}
		case 2:{
			G = (double) image[i];
			break;
		}
		case 1:{
			R = (double) image[i];
			break;
		}
		case 0:{
			A = (double) image[i];
			break;
		}
		}
	}
	PNG.close();
}

#ifdef USE_SDL
void saveFrameSDL(SDL_Surface* screen,
		int frameCounter,
		std::string movieFolder,
		std::string framePrefix)
{
	std::string frame_name = movieFolder + "/" + framePrefix + "_" +  boost::lexical_cast<std::string>(frameCounter) + ".png";
	std::cout << "Writing png to: " << frame_name << std::endl;
	int width = screen->w;
	int height = screen->h;
	int bytesPerPixel = screen->format->BytesPerPixel;
	unsigned char *image = (unsigned char*)screen->pixels;
	saveFrame(image, width, height, bytesPerPixel, frame_name);
}
#endif

#endif /* MODULES_PNGWRITER_PNGWRITER_UTIL_HPP_ */
