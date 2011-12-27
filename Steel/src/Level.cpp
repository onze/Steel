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

#include <iostream>
#include <map>
#include <set>

#include <json/json.h>

#include <OGRE/Ogre.h>

#include "Level.h"
#include "Model.h"

namespace Steel
{

Level::Level(File path, Ogre::String name, Ogre::SceneManager *sceneManager, Camera *camera) :
		mPath(path.subdir(name)), mName(name), mSceneManager(sceneManager), mResGroupAux(0), mCamera(camera)
{
	Debug::log("Level::Level(path=")(mPath)(")").endl();
	addAuxiliaryResourceName(mName);
	mLevelRoot = mSceneManager->getRootSceneNode()->createChildSceneNode("LevelNode", Ogre::Vector3::ZERO);
	mAgents = std::map<AgentId, Agent *>();
	mOgreModelMan = new OgreModelManager(mSceneManager, mLevelRoot);
}

Level::~Level()
{
	Debug::log("Level::~Level()").endl();
	mOgreModelMan->clear();
	Ogre::ResourceGroupManager *rgm = Ogre::ResourceGroupManager::getSingletonPtr();
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
	Ogre::String name = baseName + "__aux_name_#__" + Ogre::StringConverter::toString(mResGroupAux++);
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

bool Level::linkAgentToModel(AgentId aid, ModelType mtype, ModelId mid)
{
	Agent *agent = getAgent(aid);
	if (agent == NULL)
	{
		Debug::error("Steel::Level::linkAgentToOgreModel(): aid ")(aid)(" does not exist.").endl();
		return false;
	}

	ModelManager *mm = modelManager(mtype);
	if (mm == NULL)
	{
		Debug::error("Steel::Level::linkAgentToOgreModel(): mtype ")(mtype)(" aka \"");
		Debug::error(modelTypesAsString[mtype])("\" does not exist.").endl();
		return false;
	}
	//all clear
	return agent->linkToModel(mtype, mid);
}

File Level::getSavefile()
{
	return mPath.subfile(mName + ".lvl");
}

bool Level::load()
{
	Debug::log("Level::load()").endl();

	File savefile = getSavefile();
	if (!savefile.exists())
	{
		Debug::error("Level<")(mPath)(">.save(): file does not exists: ")(savefile).endl();
		return false;
	}
	Ogre::String s = savefile.read();

	if (!deserialize(s))
		return false;
	Debug::log("Level::load() loaded ")(savefile)(" successfully.").endl();
	return true;
}

bool Level::save()
{
	Debug::log("Level::save():").endl();

	Ogre::String s;
	serialize(s);

	File savefile = getSavefile();
	savefile.write(s);

	Debug::log("Level::save() into ")(savefile).endl();
	return true;
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

AgentId Level::newAgent()
{

	Agent *t = new Agent(this);
	//	Debug::log(" with id:")(t->id()).endl();
	mAgents.insert(std::pair<AgentId, Agent *>(t->id(), t));
	return t->id();
}

ModelId Level::newOgreModel(Ogre::String meshName, Ogre::Vector3 pos, Ogre::Quaternion rot, bool involvesNewResources)
{
	Debug::log("Level::newOgreModel(): meshName: ")(meshName)(" pos:")(pos)(" rot: ")(rot).endl();

#ifdef DEBUG
	assert(mSceneManager);
	assert(mLevelRoot);
#endif

	if (involvesNewResources)
		addAuxiliaryResourceName(mName);
	ModelId mid = mOgreModelMan->newModel(meshName, pos, rot);
	return mid;
}

Agent *Level::getAgent(AgentId id)
{
	std::map<AgentId, Agent *>::iterator it = mAgents.find(id);
	if (it == mAgents.end())
		return NULL;
	return it->second;
}

void Level::getAgentsIdsFromSceneNodes(std::list<Ogre::SceneNode *> &nodes, std::list<ModelId> &selection)
{
	Debug::log("Level::getAgentsIdsFromSceneNodes()").endl();
	//retrieving models
	ModelId id;
	//	Debug::log("adding ids:");
	std::list<ModelId> models = std::list<ModelId>();
	for (std::list<Ogre::SceneNode *>::iterator it = nodes.begin(); it != nodes.end(); ++it)
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
	for (std::map<AgentId, Agent *>::iterator it_agents = mAgents.begin(); it_agents != mAgents.end(); ++it_agents)
	{
		t = (*it_agents).second;
		modelId = t->ogreModelId();
		for (std::list<ModelId>::iterator it_models = models.begin(); it_models != models.end(); ++it_models)
			if (mOgreModelMan->isValid(modelId) && modelId == (*it_models))
				selection.push_back(t->id());
	}
	//	Debug::log("done.");
}

bool Level::isOver(void)
{
	return false;
}

void Level::serialize(Ogre::String &s)
{
	Debug::log("Level::serialize()").endl();
	Json::Value root;
	root["name"] = mName;

	root["camera"] = mCamera->toJson();

	// serialize agents
	Debug::log("serializing agents...").endl();
	Json::Value agents;

	for (std::map<AgentId, Agent *>::iterator it_agents = mAgents.begin(); it_agents != mAgents.end(); ++it_agents)
	{
		AgentId aid = (*it_agents).first;
		Agent *agent = (*it_agents).second;
		Debug::log("agent #")(aid)(" with ")(agent->modelsIds().size())(" modelTypes registered.").endl();

		// get agent's model ids for each type
		std::map<ModelType, ModelId> agentModelIds = agent->modelsIds();
		// add the agent's model ids to its json representation
		for (std::map<ModelType, ModelId>::iterator it_models = agentModelIds.begin(); it_models != agentModelIds.end();
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
			agents[Ogre::StringConverter::toString(aid)][modelTypesAsString[modelType]] =
					Json::Value(Ogre::StringConverter::toString(modelId));
		}
	}
	root["agents"] = agents;

	// serialize models
	Debug::log("saving model...").endl();
	Json::Value models;
	for (ModelType modelType = (ModelType) ((int) MT_FIRST + 1); modelType != MT_LAST;
			modelType = (ModelType) ((int) modelType + 1))
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

bool Level::deserialize(Ogre::String &s)
{
	Debug::log("Level::deserialize()").endl();
	Debug::log(s).endl();
	Json::Reader reader;
	Json::Value root;
	bool parsingOk = reader.parse(s, root, false);
	if (!parsingOk)
	{
		Debug::error("Level::deserialize(): could not parse this:").endl();
		Debug::error(s).endl();
		Debug::error("error is:").endl()(reader.getFormattedErrorMessages()).endl();
		return false;
	}
	Json::Value value;

	// get level info
	value = root["name"];
	assert(!value.isNull());
	assert(mName==value.asString());

	//camera
	mCamera->fromJson(root["camera"]);

	Debug::log("instanciate ALL the models ! \\o/").endl();
	Json::Value dict = root["models"];
	assert(!dict.isNull());

	for (ModelType i = (ModelType) ((int) MT_FIRST + 1); i != MT_LAST; i = (ModelType) ((int) i + 1))
	{
		Ogre::String type = modelTypesAsString[i];
		Json::Value value = dict[type];
		if (value.isNull())
		{
			Debug::log("no models for type ")(type).endl();
			continue;
		}
		ModelManager *mm = modelManager(i);
		if (mm == NULL)
		{
			Debug::warning("no modelManager for type ")(type).endl();
			continue;
		}
		mm->fromJson(value);
	}
	Debug::log("done").endl();

	Debug::log("now instanciate ALL the agents ! \\o/").endl();
	dict = root["agents"];
	assert(!dict.isNull());

	for (Json::ValueIterator it = dict.begin(); it != dict.end(); ++it)
	{
		//TODO: remap ids to lowest range
		AgentId aid = newAgent();
		assert(aid!=INVALID_ID);
		Agent *agent = getAgent(aid);
		agent->fromJson(*it);
	}
	Debug::log("done").endl();
	Debug::log("deserialization done").endl();
	return true;
}

}
