#include "gui_settings.h"
#include <JetEngine/TerrainSystem.h>

gui_settings::gui_settings(CGame* game)
{
	this->tabs.setsize(400,310);
	auto gtab = this->tabs.AddTab("Graphics"); 

	this->game = game;
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
	controller->callback = [game](gui_dropbox* box)
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
	box->callback = [game](gui_dropbox* box)
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
	box->callback = [game](gui_dropbox* box)
	{
		if (box->GetSelected() == "Off")
			game->ProcessCommand("cl_vsync 0");
		else
			game->ProcessCommand("cl_vsync 1");
		printf("Selected: %s\n", box->GetSelected().c_str());
	};
	box->SetSelected(game->GetSettingBool("cl_vsync") ? 0 : 1);
	gtab->AddWindow(box);

	//LODboost

	box = new gui_dropbox;
	box->setpos(150, 160);
	box->setsize(100, 40);
	box->AddItem("Off");
	box->AddItem("2xMSAA");
	box->AddItem("4xMSAA");
	box->settext("AA");
	box->callback = [game](gui_dropbox* box)
	{
		if (box->GetSelected() == "Off")
			game->ProcessCommand("cl_aa_samples 1");
		else if (box->GetSelected() == "2xMSAA")
			game->ProcessCommand("cl_aa_samples 2");
		else
			game->ProcessCommand("cl_aa_samples 4");
		printf("Selected: %s\n", box->GetSelected().c_str());
	};
	box->SetSelected(game->GetSettingBool("cl_aa_samples") ? 0 : 1);
	gtab->AddWindow(box);

	box = new gui_dropbox;
	box->setpos(210, 60);
	box->setsize(100, 40);
	box->AddItem("Low");
	box->AddItem("Mid");
	box->AddItem("High");
	
	box->settext("Terrain Quality");
	box->callback = [game](gui_dropbox* box)
	{
		if (box->GetSelected() == "Low")
			LODboost = 0;
		else if (box->GetSelected() == "Mid")
			LODboost = 2;
		else
			LODboost = 4;
		printf("Selected: %s\n", box->GetSelected().c_str());
	};
	box->SetSelected(1);// game->GetSettingBool("cl_vsync") ? 0 : 1);
	gtab->AddWindow(box);

	label = new gui_label;
	label->setsize(80,40);
	label->setpos(20,40);
	label->settext("Volume");
	label->AlignRight(false);
	stab->AddWindow(label);
	this->volume.setpos(130,40);
	this->volume.setsize(150,30);
	this->volume.SetRange(0,100);
	this->volume.position = SoundManager::GetInstance()->GetMasterVolume()*100.0f;
	this->volume.callback = [game](int volume)
	{
		std::string command = "cl_volume " + std::to_string(volume);
		game->ProcessCommand(command.c_str());
		SoundManager::GetInstance()->SetMasterVolume((float)volume/100.0f);
	};
	stab->AddWindow(&this->volume);

	label = new gui_label;
	label->setsize(120,40);
	label->setpos(25,120);
	label->settext("Shadow Dist");
	label->AlignRight(false);
	gtab->AddWindow(label);
	this->shadowdist.setpos(180,120);
	this->shadowdist.setsize(150,30);
	this->shadowdist.SetRange(0,300);
	this->shadowdist.position = game->GetSettingFloat("cl_shadow_dist");
	this->shadowdist.callback = [game](int volume)
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
	this->button.callback = [game](){ game->SaveSettings(); log("hit ok button\n");};
	this->AddWindow(&this->button);
}
