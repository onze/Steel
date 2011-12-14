/*
 * Level.cpp
 *
 *  Created on: 2011-05-13
 *      Author: onze
 */

#ifdef DEBUG
#include <assert.h>
#include <Debug.h>
#endif

#include <fstream>
#include <iostream>
#include <map>
#include <set>

#include <json/json.h>

#include <OGRE/Ogre.h>

#include "Level.h"
#include "Model.h"

namespace Steel
{

Level::Level(File path, Ogre::String name, Ogre::SceneManager *sceneManager) :
		mPath(path.subdir(name)), mName(name), mSceneManager(sceneManager), mResGroupAux(0)
{
	Debug::log("Level::Level(path=")(mPath)(")").endl();
	addAuxiliaryResourceName(mName);
	mLevelRoot =
			mSceneManager->getRootSceneNode()->createChildSceneNode("LevelNode",
																	Ogre::Vector3::ZERO);
	mAgents = std::map<AgentId, Agent *>();
	mOgreModelMan = new OgreModelManager(mSceneManager, mLevelRoot);
}

Level::~Level()
{
	Debug::log("Level::~Level()").endl();
	mOgreModelMan->clear();
	Ogre::ResourceGroupManager *rgm =
			Ogre::ResourceGroupManager::getSingletonPtr();
	Ogre::String name;
	for (unsigned int i = 0; i < mResGroupAux; ++i)
	{
		name = mName + "__aux_name_#__" + Ogre::StringConverter::toString(i);
		rgm->destroyResourceGroup(name);
	}
	delete mOgreModelMan;
}

Ogre::String Level::addAuxiliaryResourceName(Ogre::String baseName)
{
	Debug::log("Level::addAuxiliaryResourceName(): ")(baseName).endl();
	Ogre::String name = baseName + "__aux_name_#__"
			+ Ogre::StringConverter::toString(mResGroupAux++);
	Ogre::ResourceGroupManager::getSingletonPtr()->addResourceLocation(	mPath.subdir("materials"),
																		"FileSystem",
																		name,
																		false);
	Ogre::ResourceGroupManager::getSingletonPtr()->addResourceLocation(	mPath.subdir("meshes"),
																		"FileSystem",
																		name,
																		false);
	Ogre::ResourceGroupManager::getSingletonPtr()->initialiseResourceGroup(name);
	return name;
}

void Level::deleteAgent(AgentId id)
{
	Debug::log("Level::deleteThing() id: ")(id).endl();
	std::map<AgentId, Agent *>::iterator it = mAgents.find(id);
	if (it == mAgents.end())
		return;
	delete (*it).second;
	mAgents.erase(it);
}

File Level::getSavefilePath()
{
	return mPath.subfile(mName + ".lvl");
}

void Level::load()
{
	Debug::log("Level::load()").endl();
	File file=getSavefilePath();
	if (file.exists())
		Debug::log("file \"")(file.fullPath())("\" exists.").endl();
	else
		Debug::log("file \"")(file.fullPath())("\" does not exists.").endl();
}

ModelManager *Level::modelManager(ModelType modelType)
{
	//TODO: replace this by a lookup into a protected map of model managers.
	ModelManager *mm = NULL;
	switch (modelType)
	{
	case MT_OGRE:
		mm = mOgreModelMan;
		break;
	default:
		break;
	}
	return mm;
}

AgentId Level::newAgent(Ogre::String meshName,
						Ogre::Vector3 pos,
						Ogre::Quaternion rot,
						bool involvesNewResources)
{
	Debug::log("Level::newAgent(): meshName: ")(meshName)(" pos:")(pos)(" rot: ")(rot);

#ifdef DEBUG
	assert(mSceneManager);
	assert(mLevelRoot);
#endif

	Agent *t = new Agent(this);
	Debug::log(" with id:")(t->id()).endl();
	if (involvesNewResources)
		addAuxiliaryResourceName(mName);
	t->addModel(MT_OGRE, mOgreModelMan->newModel(meshName, pos, rot));
	mAgents.insert(std::pair<AgentId, Agent *>(t->id(), t));
	return t->id();
}

Agent *Level::getAgent(AgentId id)
{
	std::map<AgentId, Agent *>::iterator it = mAgents.find(id);
	if (it == mAgents.end())
		return NULL;
	return it->second;
}

void Level::getAgentsIdsFromSceneNodes(	std::list<Ogre::SceneNode *> &nodes,
										std::list<ModelId> &selection)
{
	Debug::log("Level::getAgentsIdsFromSceneNodes()").endl();
	//retrieving models
	ModelId id;
	//	Debug::log("adding ids:");
	std::list<ModelId> models = std::list<ModelId>();
	for (std::list<Ogre::SceneNode *>::iterator it = nodes.begin();
			it != nodes.end(); ++it)
	{
		id = Ogre::any_cast<ModelId>((*it)->getUserAny());
		if (!mOgreModelMan->isValid(id))
			continue;
		//		Debug::log("scenenode: ")((*it)->getName())("-> model id: ")(id)(", ");
		models.push_back(id);
	}
	//	Debug::log.endl()("then mapping models to agents: ");

	//retrieving agents
	Agent *t;
	ModelId modelId;
	for (std::map<AgentId, Agent *>::iterator it_agents = mAgents.begin();
			it_agents != mAgents.end(); ++it_agents)
	{
		t = (*it_agents).second;
		modelId = t->ogreModelId();
		for (std::list<ModelId>::iterator it_models = models.begin();
				it_models != models.end(); ++it_models)
			if (mOgreModelMan->isValid(modelId) && modelId == (*it_models))
				selection.push_back(t->id());
	}
	//	Debug::log("done.");
}

bool Level::isOver(void)
{
	return false;
}

void Level::save()
{
	Debug::log("Level::save():").endl();

	Ogre::String s, filename = getSavefilePath();
	serialize(s);

	std::ofstream outfile(	filename.c_str(),
							std::ios::out | std::ofstream::binary);
	outfile.write(s.c_str(), s.size());
	outfile.close();
	Debug::log("Level::save() into ")(filename).endl();
}

void Level::serialize(Ogre::String &s)
{

	Json::Value root;
	root["name"] = mName;

	// serialize agents
	Debug::log("saving agents...").endl();
	Json::Value agents;

	for (std::map<AgentId, Agent *>::iterator it_agents = mAgents.begin();
			it_agents != mAgents.end(); ++it_agents)
	{
		AgentId agentId = (*it_agents).first;
		Agent *agent = (*it_agents).second;
		Debug::log("agent #")(agentId).endl();

		//get agent's model ids for each type
		std::map<ModelType, ModelId> agentModelIds = agent->modelsIds();
		//add the agent's model ids to its json representation
		for (std::map<ModelType, ModelId>::iterator it_models =
				agentModelIds.begin(); it_models != agentModelIds.end();
				++it_models)
		{
			ModelType modelType = (*it_models).first;
			Debug::log("model type:")(modelTypesAsString[modelType]).endl();
			if ((unsigned) modelType >= modelTypesAsString.size())
			{
				Debug::error("ERROR in Level.save(): unknown ModelType")(modelType).endl();
				continue;
			}
			ModelId modelId = (*it_models).second;
			agents[Ogre::StringConverter::toString(agentId)][modelTypesAsString[modelType]] =
					Json::Value(Ogre::StringConverter::toString(modelId));
		}
	}
	root["agents"] = agents;

	// serialize models
	Debug::log("saving model...").endl();
	Json::Value models;
	for (ModelType modelType = (ModelType) ((int) MT_FIRST + 1);
			modelType != MT_LAST; modelType = (ModelType) ((int) modelType + 1))
	{
		Debug::log("processing type ")(modelTypesAsString[modelType]).endl();
		ModelManager *mm = modelManager(modelType);
		if (mm == NULL)
		{
			Debug::log("no modelManager of type ")(modelTypesAsString[modelType]).endl();
			continue;
		}
		mm->toJson(models[modelTypesAsString[modelType]]);
	}
	root["models"] = models;

	s = root.toStyledString();
	Debug::log(s).endl();
}

void Level::deserialize(Ogre::String &s)
{

}

}
