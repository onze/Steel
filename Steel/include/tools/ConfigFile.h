#ifndef STEEL_CONFIGFILE_H
#define STEEL_CONFIGFILE_H

#include <json/json.h>
#include "File.h"

namespace Steel
{

    class ConfigFile
    {
            static const Ogre::String VERSION_ATTRIBUTE_NAME;
            ConfigFile()
            {
            }
            ;
        public:
            /// Protocol version number.
            static const Ogre::String VERSION;

            ConfigFile(Steel::File file);
            ConfigFile(const ConfigFile& other);
            virtual ~ConfigFile();
            virtual ConfigFile& operator=(const ConfigFile& other);
            virtual bool operator==(const ConfigFile& other) const;

            Ogre::String getSetting(Ogre::String const &key, Ogre::String const &defaultValue="") const;
            int getSettingAsBool(Ogre::String const &key, bool defaultValue=false) const;
            int getSettingAsInt(Ogre::String const &key, int defaultValue=0) const;
            float getSettingAsFloat(Ogre::String const &key, float defaultValue=.0f) const;
            unsigned long getSettingAsUnsignedLong(Ogre::String const &key, unsigned long defaultValue=0UL) const;

            ConfigFile &setSetting(Ogre::String key, Ogre::String const &value);
            ConfigFile &setSetting(Ogre::String key, Json::Value const &value);

            void load();
            void save();

            inline File& file()
            {
                return mFile;
            }

            operator Ogre::String()
            {
                return "<ConfigFile file=" + Ogre::String(mFile) + ">";
            }

        protected:
            Json::Value &settings();
            File mFile;
            Json::Value mSettings;
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
