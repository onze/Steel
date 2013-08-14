#ifndef STEEL_BTDEBUGPRINTER_H
#define STEEL_BTDEBUGPRINTER_H

#include <OgreString.h>

#include <BT/BTNode.h>

namespace Steel
{
    /**
     * This node prints whatever content was set at build time. It is always sucessful.
     */
    class BTDebugPrinter:public BTNode
    {
    public:
        static const char *TEXT_ATTRIBUTE;
        
        inline static BTShapeTokenType tokenType()
        {
            return BTDebugPrinterToken;
        }
        BTDebugPrinter(BTShapeToken const &token);
        
        void run(float timestep);
        
    private:
        bool parseNodeContent(Json::Value &root);
        virtual ~BTDebugPrinter();
        
        Ogre::String mDebugText;
    };
}

#endif // STEEL_BTSIGNALLISTENER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
