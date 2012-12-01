#ifndef FILEEVENTLISTENER_H
#define FILEEVENTLISTENER_H

namespace Steel
{
    class File;
    class FileEventListener
    {
        public:
            virtual void onFileChangeEvent(File *file)=0;
    };
}

#endif // FILEEVENTLISTENER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
