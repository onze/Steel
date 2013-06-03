#ifndef STEEL_BTFINDER_H_
#define STEEL_BTFINDER_H_

#include <steeltypes.h>
#include <BT/btnodetypes.h>
#include <BT/BTNode.h>
#include <AgentSpec.h>

namespace Steel
{
    class BTFinder: public BTNode
    {
        public:
            static const Ogre::String LOCALIZATION_MODE;
            static const Ogre::String LOCALIZATION_MODE_STATIC;
            static const Ogre::String LOCALIZATION_MODE_DYNAMIC;
            
            /// How to localize the target agent.
            enum LocalizationMode
            {
                /// The agent is localized once, the first time the BTFinder node is queried.
                LM_STATIC=0,
                /// The agent is localized each time the BTFinder node is queried.
                LM_DYNAMIC
            };

            BTFinder(BTShapeToken const &token);
            virtual ~BTFinder();
            
            inline static BTShapeTokenType tokenType()
            {
                return BTFinderToken;
            }
            
            inline AgentSpec &agentSpec()
            {
                return mAgentSpec;
            }
            
            inline LocalizationMode localizationMode() const
            {
                return mLocalizationMode;
            }


        private:

            /// See BTNode::parseNodeContent
            bool parseNodeContent(Json::Value &root);
            //notowned
            // owned
            /// what to look for
            AgentSpec mAgentSpec;
            
            LocalizationMode mLocalizationMode;
    };

}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
