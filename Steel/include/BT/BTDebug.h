#ifndef STEEL_BTDEBUGPRINTER_H
#define STEEL_BTDEBUGPRINTER_H

#include <OgreString.h>

#include <BT/BTNode.h>

namespace Steel
{
    /**
     * This node prints whatever content was set at build time. It is always sucessful.
     */
    class BTDebug:public BTNode
    {
        public:
            static const char *TEXT_ATTRIBUTE;

            inline static BTShapeTokenType tokenType()
            {
                return BTDebugToken;
            }
            BTDebug(BTShapeToken const &token);
            virtual ~BTDebug();

            void run(float timestep);

        private:
            bool parseNodeContent(Json::Value &root);

            Ogre::String mDebugText;
    };
}

#endif // STEEL_BTSIGNALLISTENER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
