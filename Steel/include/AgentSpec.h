#ifndef STEEL_AGENTSPEC_H
#define STEEL_AGENTSPEC_H

#include <json/json.h>
#include <OgrePrerequisites.h>

namespace Steel
{
    /**
     * Describes a query that can match an agent.
     */
    class AgentSpec
    {
        public:
            static const Ogre::String TAG;

            AgentSpec();
            AgentSpec(const AgentSpec& other);
            virtual ~AgentSpec();
            virtual AgentSpec& operator=(const AgentSpec& other);
            virtual bool operator==(const AgentSpec& other) const;

            /// Sets spec according to Json content. Returns false if errors occured.
            bool parseJson(const Json::Value &value);
            
            inline Ogre::String tag() const
            {
                return mTag;
            }

        private:
            Ogre::String mTag;
    };

}

#endif // STEEL_AGENTSPEC_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
