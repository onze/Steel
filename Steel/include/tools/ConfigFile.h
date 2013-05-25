#ifndef STEEL_CONFIGFILE_H
#define STEEL_CONFIGFILE_H

#include "File.h"

namespace Steel
{

    class ConfigFile
    {
            static const Ogre::String VERSION_ATTRIBUTE_NAME;
            ConfigFile() {};
        public:
            /// Protocol version number.
            static const Ogre::String VERSION;

            ConfigFile(Steel::File file, bool autoLoad = true);
            ConfigFile(const ConfigFile& other);
            virtual ~ConfigFile();
            virtual ConfigFile& operator=(const ConfigFile& other);
            virtual bool operator==(const ConfigFile& other) const;

            Ogre::String getSetting(Ogre::String key);
            ConfigFile &setSetting(Ogre::String key, Ogre::String value);

            void load();
            void save();

            inline File& file()
            {
                return mFile;
            }

            operator Ogre::String()
            {
                return "<ConfigFile file="+Ogre::String(mFile)+">";
            }
            
        protected:
            File mFile;
            Ogre::NameValuePairList mSettings;
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
