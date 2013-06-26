/*
 *				����: �
 *				����: 2012-12-18
 *				����: URL·��
 *				����:	 ����HttpRequest�����URL������Ӧ��Controller��Ȼ�������Ӧ��Action������
 */
#ifndef __ROUTER_H__
#define __ROUTER_H__

#include <string>

NS_DRAGON 
		class Application;

		typedef struct 
		{
			std::string ControllerName;
			std::string ActionName;
			std::string Parameters;
			std::string OptionParameter;
		} routing_table_t;

		class Router
		{
		private:
			Application *app_;
			static std::map<std::string, std::string> routingMap;

			bool buildRoutingTable(routing_table_t *rt, std::string url);

		public:
			Router()  : app_(NULL)  {}
			~Router() {}

			static void Init();

			Application *getApp()
			{
				return app_;
			}

			void setApp(Application *app)
			{
				app_ = app;
			}

			void route(HttpRequest &request, 
							  HttpResponse &response);
		};

NS_END

#endif
