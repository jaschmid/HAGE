#include "header.h"
#include "SettingsLoader.h"

#include <fstream>

namespace HAGE{
	
	SettingsLoader* settings;
	void initSettings(){
		settings = new SettingsLoader();
	}	
	void deleteSettings(){
		delete settings;
	}
	
	SettingsLoader::SettingsLoader(){
		std::cout << "Loading settings";
		std::ifstream file("config.hcf"); //hfc = hage config
		assert(file.is_open());
		assert(!file.eof());
		
		std::string a;
		file >> a;
		while(!file.eof()){
			if(strcmp(a.c_str(),"string") == 0){
				file >> a;
				file >> stringSettings[a];
			}
			else if(strcmp(a.c_str(),"f32") == 0){
				file >> a;
				file >> f32Settings[a];
			}
			else if(strcmp(a.c_str(),"f64") == 0){
				file >> a;
				file >> f64Settings[a];
			}
			else if(strcmp(a.c_str(),"u64") == 0){
				file >> a;
				file >> u64Settings[a];
			}
			else if(strcmp(a.c_str(),"u32") == 0){
				file >> a;
				file >> u32Settings[a];
			}
			else if(strcmp(a.c_str(),"u16") == 0){
				file >> a;
				file >> u16Settings[a];
			}
			else if(strcmp(a.c_str(),"u8") == 0){
				file >> a;
				file >> u8Settings[a];
			}
			else if(strcmp(a.c_str(),"i64") == 0){
				file >> a;
				file >> i64Settings[a];
			}
			else if(strcmp(a.c_str(),"i32") == 0){
				file >> a;
				file >> i32Settings[a];
			}
			else if(strcmp(a.c_str(),"i16") == 0){
				file >> a;
				file >> i16Settings[a];
			}
			else if(strcmp(a.c_str(),"i8") == 0){
				file >> a;
				file >> i8Settings[a];
			}
			else if(strcmp(a.c_str(),"bool") == 0){
				file >> a;
				std::string b;
				file >> b;
				if(b == "true")
					boolSettings[a]=true;
				else if(b == "false")
					boolSettings[a]=false;
				else
					assert(!"Unknown boolean expression");
			}else{
				std::cout << "...Setting loading error: Unkown data type " << a <<" Error might occured while application is running" << std::endl;
				assert(false);
			}
			file >> a;	
		}

		file.close();

		std::cout << "...settings loaded" << std::endl;
	}

	bool SettingsLoader::getBoolSetting(std::string name){
		assert(boolSettings.count(name)!=0);
		return boolSettings[name];
	}
	f32 SettingsLoader::getf32Setting(std::string name){
		assert(f32Settings.count(name)!=0);
		return f32Settings[name];
	}
	f64 SettingsLoader::getf64Setting(std::string name){
		assert(f64Settings.count(name)!=0);
		return f64Settings[name];
	}
	u64 SettingsLoader::getu64Setting(std::string name){
		assert(u64Settings.count(name)!=0);
		return u64Settings[name];
	}
	u32 SettingsLoader::getu32Setting(std::string name){
		assert(u32Settings.count(name)!=0);
		return u32Settings[name];
	}
	u16 SettingsLoader::getu16Setting(std::string name){
		assert(u16Settings.count(name)!=0);
		return u16Settings[name];
	}
	u8  SettingsLoader::getu8Setting (std::string name){
		assert(u8Settings.count(name)!=0);
		return u8Settings[name];
	}
	i64 SettingsLoader::geti64Setting(std::string name){
		assert(i64Settings.count(name)!=0);
		return i64Settings[name];
	}
	i32 SettingsLoader::geti32Setting(std::string name){
		assert(i32Settings.count(name)!=0);
		return i32Settings[name];
	}
	i16 SettingsLoader::geti16Setting(std::string name){
		assert(i16Settings.count(name)!=0);
		return i16Settings[name];
	}
	i8  SettingsLoader::geti8Setting (std::string name){
		assert(i8Settings.count(name)!=0);
		return i8Settings[name];
	}
	std::string SettingsLoader::getstringSetting (std::string name){
		assert(stringSettings.count(name)!=0);
		return stringSettings[name];
	}

	void SettingsLoader::setBoolSetting(std::string name,bool value){
		boolSettings[name] = value;
	}
	void SettingsLoader::setf32Setting(std::string name,f32 value){
		f32Settings[name] = value;
	}
	void SettingsLoader::setf64Setting(std::string name,f64 value){
		f64Settings[name] = value;
	}
	void SettingsLoader::setu64Setting(std::string name,u64 value){
		u64Settings[name] = value;
	}
	void SettingsLoader::setu32Setting(std::string name,u32 value){
		u32Settings[name] = value;
	}
	void SettingsLoader::setu16Setting(std::string name,u16 value){
		u16Settings[name] = value;
	}
	void SettingsLoader::setu8Setting (std::string name,u8 value){
		u8Settings[name] = value;
	}
	void SettingsLoader::seti64Setting(std::string name,i64 value){
		i64Settings[name] = value;
	}
	void SettingsLoader::seti32Setting(std::string name,i32 value){
		i32Settings[name] = value;
	}
	void SettingsLoader::seti16Setting(std::string name,i16 value){
		i16Settings[name] = value;
	}
	void SettingsLoader::seti8Setting (std::string name,i8 value){
		i8Settings[name] = value;
	}
	void SettingsLoader::setstringSetting (std::string name,std::string value){
		stringSettings[name] = value;
	}

}