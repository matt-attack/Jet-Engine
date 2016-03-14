#include "gui_settings.h"

gui_settings::gui_settings() 
{
	this->tabs.setsize(400,310);
	auto gtab = this->tabs.AddTab("Graphics"); 
	//gui_button* button = new gui_button;
	//button->setpos(50,50);
	//button->setsize(100,40);
	//button->settext("Graphics");
	//gtab->AddWindow(button);
	auto stab = this->tabs.AddTab("Sound");
	auto ctab = this->tabs.AddTab("Controls");
	auto label = new gui_label;
	label->setsize(300,400);
	label->setpos(20,20);
	label->settext("These are the controls:\n"
		"Movement - WASD\n"
		"Weapon Selection - Number Keys\n"
		"Give Order - O\n"
		"Chat - Y\n"
		"Reload -R\n"
		"Enter Vehicle - E\n"
		"Third Person - T");
	label->AlignRight(false);
	ctab->AddWindow(label);

	label = new gui_label;
	label->setpos(10, 200);
	label->setsize(120, 20);
	label->settext("First Player Controller");
	ctab->AddWindow(label);

	auto controller = new gui_dropbox;
	controller->setpos(150, 200);
	controller->setsize(100, 40);
	controller->AddItem("On");
	controller->AddItem("Off");
	controller->callback = [](gui_dropbox* box)
	{
		if (box->GetSelected() == "Off")
			game->ProcessCommand("cl_controller 0");
		else
			game->ProcessCommand("cl_controller 1");
		printf("Selected: %s\n", box->GetSelected().c_str());
	};
	controller->SetSelected(game->GetSettingBool("cl_controller") ? 0 : 1);
	ctab->AddWindow(controller);
	//button = new gui_button;
	//button->setpos(150,50);
	//button->setsize(100,40);
	//button->settext("Controls");
	//ctab->AddWindow(button);
	tabs.setpos(0,31);
	this->AddWindow(&tabs);
	//add changable settings
	this->setsize(400,400);

	//auto t = new gui_textbox;
	//t->setsize(100,40);
	//t->setpos(150,150);
	//ctab->AddWindow(t);

	gui_dropbox* box = new gui_dropbox;
	box->setpos(150,10);
	box->setsize(100,40);
	box->AddItem("On");
	box->AddItem("Off");
	box->settext("Shadows");
	box->callback = [](gui_dropbox* box)
	{
		if (box->GetSelected() == "Off")
			game->ProcessCommand("cl_shadows 0");
		else
			game->ProcessCommand("cl_shadows 1");
		printf("Selected: %s\n", box->GetSelected().c_str());
	};
	box->SetSelected(game->GetSettingBool("cl_shadows") ? 0 : 1);
	gtab->AddWindow(box);


	box = new gui_dropbox;
	box->setpos(150,60);
	box->setsize(100,40);
	box->AddItem("On");
	box->AddItem("Off");
	box->settext("Vsync");
	box->callback = [](gui_dropbox* box)
	{
		if (box->GetSelected() == "Off")
			game->ProcessCommand("cl_vsync 0");
		else
			game->ProcessCommand("cl_vsync 1");
		printf("Selected: %s\n", box->GetSelected().c_str());
	};
	box->SetSelected(game->GetSettingBool("cl_vsync") ? 0 : 1);
	gtab->AddWindow(box);

	label = new gui_label;
	label->setsize(80,40);
	label->setpos(20,40);
	label->settext("Volume");
	label->AlignRight(false);
	stab->AddWindow(label);
	this->volume.setpos(130,40);
	this->volume.setsize(150,30);
	this->volume.SetRange(100);
	this->volume.position = SoundManager::GetInstance()->GetMasterVolume()*100.0f;
	this->volume.callback = [](int volume)
	{
		std::string command = "cl_volume " + std::to_string(volume);
		game->ProcessCommand(command.c_str());
		SoundManager::GetInstance()->SetMasterVolume((float)volume/100.0f);
	};
	stab->AddWindow(&this->volume);

	label = new gui_label;
	label->setsize(120,40);
	label->setpos(25,150);
	label->settext("Shadow Dist");
	label->AlignRight(false);
	gtab->AddWindow(label);
	this->shadowdist.setpos(180,150);
	this->shadowdist.setsize(150,30);
	this->shadowdist.SetRange(300);
	this->shadowdist.position = game->GetSettingFloat("cl_shadow_dist");
	this->shadowdist.callback = [](int volume)
	{
		std::string command = "cl_shadow_dist " + std::to_string(volume);
		game->ProcessCommand(command.c_str());
	};
	gtab->AddWindow(&this->shadowdist);

	//add volume settings
	//investigate chat box auto scroll
	//check distance display on other players, was displaying random numbers

	this->button.setpos(0,10);
	this->button.setsize(100,40);
	this->button.settext("Ok");
	this->button.callback = [](){ game->SaveSettings(); log("hit ok button\n");};
	this->AddWindow(&this->button);
}
