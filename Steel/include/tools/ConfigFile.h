#ifndef STEEL_CONFIGFILE_H
#define STEEL_CONFIGFILE_H

#include "File.h"

namespace Steel
{

    class ConfigFile
    {
            ConfigFile() {};
        public:
            ConfigFile(File file);
            ConfigFile(const ConfigFile& other);
            virtual ~ConfigFile();
            virtual ConfigFile& operator=(const ConfigFile& other);
            virtual bool operator==(const ConfigFile& other) const;

            Ogre::String getSetting(Ogre::String key);
            void setSetting(Ogre::String key, Ogre::String value);

            void load();
            void save();
        protected:
            File mFile;
            Ogre::NameValuePairList mSettings;
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
