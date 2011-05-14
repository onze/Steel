/*
 * Level.h
 *
 *  Created on: 2011-05-13
 *      Author: onze
 */

#ifndef LEVEL_H_
#define LEVEL_H_

#include <Engine.h>

namespace Steel
{

class Level
{
private:
	Level();
public:
	Level(Engine *engine);
	virtual ~Level();
	bool isOver();
};

}

#endif /* LEVEL_H_ */
