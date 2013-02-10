#ifndef STEEL_FILEEVENTLISTENER_H
#define STEEL_FILEEVENTLISTENER_H

namespace Steel
{
    class File;
    class FileEventListener
    {
        public:
            virtual void onFileChangeEvent(File file)=0;
    };
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
