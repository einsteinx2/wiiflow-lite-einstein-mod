
#include <ogc/system.h>
#include <unistd.h>

#include "const_str.hpp"
#include "booter/external_booter.hpp"
#include "channel/nand.hpp"
#include "channel/nand_save.hpp"
#include "devicemounter/DeviceHandler.hpp"
#include "gecko/gecko.hpp"
#include "gui/video.hpp"
#include "gui/text.hpp"
#include "homebrew/homebrew.h"
#include "loader/alt_ios_gen.h"
#include "loader/wdvd.h"
#include "loader/alt_ios.h"
#include "loader/sys.h"
#include "loader/wbfs.h"
#include "loader/cios.h"
#include "loader/nk.h"
#include "menu/menu.hpp"
#include "memory/memory.h"

bool useMainIOS = false;
volatile bool NANDemuView = false;
volatile bool networkInit = false;

/*  If we're not using USB, currently it waits up to 20 seconds before loading the UI.
	So this provides an easy check to see if we have everything set to use the SD card.
	If we do, then we can skip trying to mount USB at all. */ 
bool isUsingUSB() {
	// /* First check if the config file exists on the SD card, if not, not only are we definitely
	//    using USB (or the config file doesn't exist at all, maybe first launch) and we can't do 
	//    the other checks anyway. */
	// struct stat dummy;
	// const char *configFilePath = fmt("%s:/%s", DeviceName[SD], APPS_DIR);
	// if(DeviceHandle.IsInserted(SD) && DeviceHandle.GetFSType(SD) != PART_FS_WBFS && stat(configFilePath, &dummy) == 0)
	// {
	// 	gprintf("isUsingUSB: No config file exists on SD card, so assuming we're using USB\n");
	// 	return true;
	// }
	
	/* First check if the app path exists on the SD card, if not then we're using USB */
	struct stat dummy;
	string appPath = fmt("%s:/%s", DeviceName[SD], APPS_DIR);
	if(DeviceHandle.IsInserted(SD) && DeviceHandle.GetFSType(SD) != PART_FS_WBFS && stat(appPath.c_str(), &dummy) != 0)
	{
		gprintf("isUsingUSB: No app path exists on SD card, so assuming we're using USB\n");
		return true;
	}
	
	/* Check that the config file exists, or we can't do the following checks */
	string configPath = fmt("%s/" CFG_FILENAME, appPath.c_str());
	if(stat(configPath.c_str(), &dummy) != 0)
	{
		gprintf("isUsingUSB: The app path is on SD but no config file exists, so assuming we might need USB\n");
		return true;
	}
	
	/* Load the config file */
	Config m_cfg;// = new Config();
	if(!m_cfg.load(configPath.c_str())) 
	{
		gprintf("isUsingUSB: The app path is on SD and a config file exists, but we can't load it, so assuming we might need USB\n");
		return true;
	}
	
	/* If we have the WiiFlow data on USB, then we're using USB */
	if(m_cfg.getBool(GENERAL_DOMAIN, "data_on_usb", false))
	{
		gprintf("isUsingUSB: data_on_usb is true, so assuming we're using USB\n");
		return true;
	}
	
	/* If any of the sections have partition set > 0, we're on USB */
	const char *domains[] = {WII_DOMAIN, GC_DOMAIN, CHANNEL_DOMAIN, PLUGIN_DOMAIN, HOMEBREW_DOMAIN};
	for(int i = 0; i < 5; i++)
	{
		if(!m_cfg.getBool(domains[i], "disable", false) && m_cfg.getInt(domains[i], "partition", SD) != SD)
		{
			gprintf("isUsingUSB: %s domain is enabled and partition is not SD (i.e. greater than 0), so assuming we're using USB\n", domains[i]);
			return true;
		}
	}
	
	// if(!m_cfg.getBool(domains[WII_DOMAIN], "disable", false) && m_cfg.getInt(domains[WII_DOMAIN], "partition", SD) != SD)
	// {
	// 	gprintf("isUsingUSB: %s domain is enabled and partition is not SD (i.e. greater than 0), so assuming we're using USB\n");
	// }
	
	gprintf("isUsingUSB: we're not using USB\n");
	return false;
}

int main(int argc, char **argv)
{
	MEM_init(); //Inits both mem1lo and mem2
	mainIOS = DOL_MAIN_IOS;// 249
    m_vid.init(); // Init video
    /*  Show first frame of the loading animation by setting a long wait time. 
    	If we animate right away, it'll stall for a second while we init ISFS */
    m_vid.animateWaitMessages(false); 
    m_vid.waitMessage(0.15f);
	__exception_setreload(10);
	Gecko_Init(); //USB Gecko and SD/WiFi buffer
	gprintf(" \nWelcome to %s!\nThis is the debug output.\n", VERSION_STRING.c_str());

	DeviceHandle.Init();
	NandHandle.Init();

	char *gameid = NULL;
	bool iosOK = true;

	for(u8 i = 0; i < argc; i++)
	{
		if(argv[i] != NULL && strcasestr(argv[i], "ios=") != NULL && strlen(argv[i]) > 4)
		{
			while(argv[i][0] && !isdigit(argv[i][0]))
				argv[i]++;
			if(atoi(argv[i]) < 254 && atoi(argv[i]) > 0)
				mainIOS = atoi(argv[i]);
		}
		else if(strlen(argv[i]) == 6)
		{
			gameid = argv[i];
			for(u8 i = 0; i < 5; i++)
			{
				if(!isalnum(gameid[i]))
					gameid = NULL;
			}
		}
	}
	check_neek2o();
    gprintf("Current IOS version: %d\n", IOS_GetVersion());
	/* Init ISFS */
	if(neek2o() || Sys_DolphinMode())
		NandHandle.Init_ISFS();
	else
		NandHandle.LoadDefaultIOS(); /* safe reload to preferred IOS */
    m_vid.animateWaitMessages(true); // Start animating the loading screen
	/* Maybe new IOS and Port settings */
	if(InternalSave.CheckSave())
		InternalSave.LoadSettings();
	/* Handle (c)IOS Loading */
	if(neek2o() || Sys_DolphinMode()) /* wont reload anythin */
		iosOK = loadIOS(IOS_GetVersion(), false);
	else if(useMainIOS && CustomIOS(IOS_GetType(mainIOS))) /* Requested */
		iosOK = loadIOS(mainIOS, false) && CustomIOS(CurrentIOS.Type);
	// Init
	Sys_Init();
	Sys_ExitTo(EXIT_TO_HBC);

	DeviceHandle.MountSD();
	if(isUsingUSB())
		DeviceHandle.MountAllUSB();

	Open_Inputs();
	if(mainMenu.init())
	{
		if(CurrentIOS.Version != mainIOS && !neek2o() && !Sys_DolphinMode())
		{
			if(useMainIOS || !DeviceHandle.UsablePartitionMounted())
			{
				useMainIOS = false;
				mainMenu.TempLoadIOS();
				iosOK = CustomIOS(CurrentIOS.Type);
			}
		}
		if(CurrentIOS.Version == mainIOS)
			useMainIOS = true; //Needed for later checks
		if(!iosOK)
			mainMenu.terror("errboot1", L"No cIOS found!\ncIOS d2x 249 base 56 and 250 base 57 are enough for all your games.");
		else if(!DeviceHandle.UsablePartitionMounted())
			mainMenu.terror("errboot2", L"Could not find a device to save configuration files on!");
		else if(WDVD_Init() < 0)
			mainMenu.terror("errboot3", L"Could not initialize the DIP module!");
		else 
		{
			writeStub();
			if(gameid != NULL && strlen(gameid) == 6)
				mainMenu.directlaunch(gameid);
			else
				mainMenu.main();
				//if mainMenu.init set exit=true then mainMenu.main while loop does nothing and returns to here to exit wiiflow
		}
		//Exit WiiFlow, no game booted...
		mainMenu.cleanup();// removes all sounds, fonts, images, coverflow, plugin stuff, source menu and clear memory
	}
	ShutdownBeforeExit();// unmount devices and close inputs
	Sys_Exit();
	return 0;
}
