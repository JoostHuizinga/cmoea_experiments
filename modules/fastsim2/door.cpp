#include "door.hpp"
#include <iostream>
namespace fastsim {
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////
  ////////////* Constructors/Destructors *////////////////
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////

  /*
   * Creates a door with the given width, height, and position.
   * Doors block all movement as long as they are active.
   * If the switch attached to this door is activated, the door will be opened.
   * NOTE: the longer dimension (width or height) determines the "direction" of this door.
   * Doors with a larger width will be treated as facing north/south, while doors with a larger height will be treated as facing east/west.
   * Doors are accessible only from north/south or east/west (i.e. can only be opened from two sides).
   */
  Door::Door(boost::shared_ptr<Map> map, int color, int x, int y, int width, int height) :
  	color(color),
	x(x),
	y(y),
	width(width),
	height(height),
	active(true)
  {
  	map->drawRegion(x, y, width, height, color);
  	/* CASE 1: door is horizontally aligned */
  	if (width > height) {
  	  map->addIlluminatedSwitch(Map::illuminated_switch(new IlluminatedSwitch(color, height, x + width / 2.0, y, true)));
  	  map->addIlluminatedSwitch(Map::illuminated_switch(new IlluminatedSwitch(color, height, x + width / 2.0, y + height, true)));
  	/* CASE 2: door is vertically aligned or is a square */
  	} else {
  	  map->addIlluminatedSwitch(Map::illuminated_switch(new IlluminatedSwitch(color, width, x + width, y + height / 2.0, true)));
  	  map->addIlluminatedSwitch(Map::illuminated_switch(new IlluminatedSwitch(color, width, x, y + height / 2.0, true)));
  	}
  	switchIndex1 = map->getIlluminatedSwitches().size() - 2;
  	switchIndex2 = map->getIlluminatedSwitches().size() - 1;
  	map->getIlluminatedSwitches().at(switchIndex1)->setVisible(false);
  	map->getIlluminatedSwitches().at(switchIndex2)->setVisible(false);
  }

  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////* Functions *///////////////////////
  ////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////

  /* Updates the door; if its switch has been pressed, the door will be opened. */
  void Door::update(boost::shared_ptr<Map> map) {
    if (map->getIlluminatedSwitches().at(switchIndex1)->isActivated() || map->getIlluminatedSwitches().at(switchIndex2)->isActivated()) {
      std::cout << "HERE";
      map->drawRegion(x, y, width, height, Map::noCollision);
      map->getIlluminatedSwitches().at(switchIndex1)->deactivate();
      map->getIlluminatedSwitches().at(switchIndex1)->turnOff();
      map->getIlluminatedSwitches().at(switchIndex2)->deactivate();
      map->getIlluminatedSwitches().at(switchIndex2)->turnOff();
      active = false;
    }
  }

  /* Resets the door and associated switch to its default state (closed). */
  void Door::reset(boost::shared_ptr<Map> map) {
    map->drawRegion(x, y, width, height, color);
    active = true;
    map->getIlluminatedSwitches().at(switchIndex1)->deactivate();
    map->getIlluminatedSwitches().at(switchIndex1)->turnOn();
    map->getIlluminatedSwitches().at(switchIndex2)->deactivate();
    map->getIlluminatedSwitches().at(switchIndex2)->turnOn();
  }
}
