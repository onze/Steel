/*
 * Node.h
 *
 *  Created on: 2011-07-18
 *      Author: onze
 */

#ifndef BTNODE_H_
#define BTNODE_H_

#include <list>
#include <string>

namespace Steel
{

class BTNode
{
protected:
	BTNode(BTNode *parent = NULL);

public:
	enum BTState
	{
		READY, RUNNING, SUCCESS, FAILURE, ERROR
	};
	virtual ~BTNode();
	virtual void attachChild(BTNode *child);
	virtual BTState run()
	{
		return SUCCESS;
	}
	;
	//events
	virtual void onStartRunning()
	{
		mState = RUNNING;
	}
	virtual void onStopRunning()
	{
		mState = READY;
	}
	//getter
	BTState state()
	{
		return mState;
	}
	std::string dbgName()
	{
		return mDbgName;
	}
	//setter
	void setDbgName(std::string dbgName)
	{
		mDbgName = dbgName;
	}
protected:
	BTState mState;
	BTNode *mParent;
	std::list<BTNode *> mChildren;
	std::string mDbgName;
};

}
#endif /* BTNODE_H_ */
