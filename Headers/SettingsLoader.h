#ifndef SETTINGSLOADER_HEADER_INCLUDED
#define SETTINGSLOADER_HEADER_INCLUDED

#include "header.h"

namespace HAGE{
	
class SettingsLoader
{
	friend void initSettings();
	std::map<std::string,bool> boolSettings;
	std::map<std::string,f32> f32Settings;
	std::map<std::string,f64> f64Settings;
	std::map<std::string,u64> u64Settings;
	std::map<std::string,u32> u32Settings;
	std::map<std::string,u16> u16Settings;
	std::map<std::string,u8>  u8Settings;
	std::map<std::string,i64> i64Settings;
	std::map<std::string,i32> i32Settings;
	std::map<std::string,i16> i16Settings;
	std::map<std::string,i8>  i8Settings;
	std::map<std::string,std::string>  stringSettings;
	
	SettingsLoader();
	SettingsLoader(const SettingsLoader &){};
	~SettingsLoader(){std::cout << "deleting settings" << std::endl;}
public:

	bool getBoolSetting(std::string name);
	f32 getf32Setting(std::string name);
	f64 getf64Setting(std::string name);
	u64 getu64Setting(std::string name);
	u32 getu32Setting(std::string name);
	u16 getu16Setting(std::string name);
	u8  getu8Setting (std::string name);
	i64 geti64Setting(std::string name);
	i32 geti32Setting(std::string name);
	i16 geti16Setting(std::string name);
	i8  geti8Setting (std::string name);
	std::string getstringSetting (std::string name);

	void setBoolSetting(std::string name,bool value);
	void setf32Setting(std::string name,f32 value);
	void setf64Setting(std::string name,f64 value);
	void setu64Setting(std::string name,u64 value);
	void setu32Setting(std::string name,u32 value);
	void setu16Setting(std::string name,u16 value);
	void setu8Setting (std::string name,u8 value);
	void seti64Setting(std::string name,i64 value);
	void seti32Setting(std::string name,i32 value);
	void seti16Setting(std::string name,i16 value);
	void seti8Setting (std::string name,i8 value);
	void setstringSetting (std::string name,std::string value);
};

extern SettingsLoader *settings;

}

#endif

