#include "tools/ConfigFile.h"
#include <tools/StringUtils.h>

namespace Steel
{
    ConfigFile::ConfigFile(File file,bool autoLoad):mFile(file),mSettings(Ogre::NameValuePairList())
    {
        if(autoLoad)
            load();
    }

    ConfigFile::ConfigFile(const ConfigFile& o):mFile(o.mFile),mSettings(o.mSettings.begin(),o.mSettings.end())
    {
        load();
    }

    ConfigFile::~ConfigFile()
    {

    }

    ConfigFile& ConfigFile::operator=(const ConfigFile& o)
    {
        if(&o==this)
            return *this;
        mFile=o.mFile;
        mSettings=o.mSettings;
        return *this;
    }

    bool ConfigFile::operator==(const ConfigFile& o) const
    {
        bool equals=true;

        equals&=mFile==o.mFile;
        equals&=mSettings==o.mSettings;

        return equals;
    }

    void ConfigFile::load()
    {
        if(!mFile.exists())
            return;
        auto lines=StringUtils::split(mFile.read(),"\n");
        for(auto it=lines.begin(); it!=lines.end(); ++it)
        {
            Ogre::String line=*it;
            // comment
            if(line.size()==0 || line.at(0)=='#')
                continue;

            auto words=StringUtils::split(line,"=");
            if(words.size()==0)
                continue;

            Ogre::String key=words[0],value="true";
            if(words.size()>1)
                value=StringUtils::join(words,"=",1);
            Ogre::StringUtil::trim(key);
            Ogre::StringUtil::trim(value);
            mSettings.insert(Ogre::NameValuePairList::value_type(key,value));
        }
    }

    void ConfigFile::save()
    {
        // little backup
        File backup(mFile.fullPath()+".back");
        if(mFile.exists())
            backup.write(mFile.read(),File::OM_OVERWRITE);

        // write settings
        for(auto it=mSettings.begin(); it!=mSettings.end(); ++it)
        {
            Ogre::NameValuePairList::value_type pair=*it;
            mFile.write(pair.first+" = "+pair.second+"\n",File::OM_APPEND);
        }
        mFile.write("\n",File::OM_APPEND);
    }

    ConfigFile& ConfigFile::setSetting(Ogre::String key, Ogre::String value)
    {
        mSettings.erase(key);
        mSettings.insert(std::pair<Ogre::String,Ogre::String>(key,value));
        return *this;
    }

    Ogre::String ConfigFile::getSetting(Ogre::String key)
    {
        auto it=mSettings.find(key);
        if(it==mSettings.end())
            return "";
        return it->second;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
