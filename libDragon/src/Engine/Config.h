/*
 *				����: �
 *				����: 2012-12-18
 *				����: �����ļ�����
 *				����:	 ����Ӧ�ó���·���µ�App.config�����ļ�
*/
#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "../DragonConfig.h"

NS_DRAGON
	/*
			�����ļ���ʽ
			[section1]
			key1 = value1 
			[section2]
			key2 = value2 
	*/
	typedef std::string sname_t; 
	typedef std::string key_t;
	typedef std::string value_t;
	typedef std::map<key_t, value_t> kv_t;
	typedef std::map<sname_t, kv_t> config_t;

	class Config {
	protected:
		config_t config;
		
	public:
		Config() {}
		~Config() {}

		void Parse(const char *path);

		kv_t &operator[](const sname_t &name) 
		{
			return config[name];
		}

		config_t &getConfig()
		{
			return config;
		}
	};

NS_END

#endif