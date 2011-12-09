/*
 * steeltypes.h
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#ifndef STEELTYPES_H_
#define STEELTYPES_H_

#include <limits.h>
#include <vector>

#include <OGRE/OgreString.h>

namespace Steel
{

typedef unsigned long AgentId;
typedef unsigned long ModelId;

const unsigned long INVALID_ID = ULONG_MAX;

//those two need to stay in synch, and the usable enum values need to stay contiguous starting at 0.
enum ModelType
{
	//MT_FIRST should stay first
	MT_FIRST=-1,
	//put next ones here
	MT_OGRE,
	//MT_LAST should stay last (to enable looping).
	MT_LAST
};
/**Maps a ModelType to its string representation.*/
extern std::vector<Ogre::String> modelTypesAsString;

}

#endif /* STEELTYPES_H_ */
