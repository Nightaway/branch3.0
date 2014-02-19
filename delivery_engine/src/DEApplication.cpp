#include "DEApplication.h"

#include "config.h"
#include "Controllers/deliveryController.h"

#include <iostream>

NS_USING_DRAGON

void DEApplication::Start()
{
	Application::Start();
	std::cout << "DE Start at path:" << appPath_ << std::endl;

	deliveryController::InitClass_();
}

void DEApplication::End()
{
	std::cout << "DE End" << std::endl;
	Application::End();
}

void DEApplication::SetAppPath()
{
	appPath_ = DE_APP_PATH;
}
