function Component()
{	

	//save the binary name
	installer.setValue("BinaryName", installer.value("ProductName"));
	//check if architecture is supported
    //if(!testArch())
        //return;

	//add custom pages (installer only)
	if(installer.isInstaller()) {
		//installer.addWizardPage(component, "UserPage", QInstaller.TargetDirectory);
		if (installer.value("os") === "win")// only windows -> desktop shortcut
			installer.addWizardPage(component, "ShortcutPage", QInstaller.ReadyForInstallation);
	}
}

Component.prototype.createOperations = function()
{


	try {
		component.createOperations();
		//update RunProgram, depending on the os
		if (installer.value("os") === "win") {
			installer.setValue("RunProgram", "@TargetDir@/@ProductName@.exe");
		} else if(installer.value("os") === "mac") {
			installer.setValue("RunProgram", "@TargetDir@/Contents/MacOS/@ProductName@");
		} else if(installer.value("os") === "x11") {
			installer.setValue("RunProgram", "@TargetDir@/@ProductName@");
		}
		if (installer.value("os") === "win") {
			//win -> add startmenu shortcuts
			component.addOperation("CreateShortcut", "@TargetDir@/@ProductName@.exe", "@StartMenuDir@/@Name@.lnk");
            component.addOperation("Execute", "@TargetDir@\\vc_redist.x64.exe","/install","/passive", "/norestart","/quiet");
			if(installer.isOfflineOnly())
				component.addOperation("CreateShortcut", "@TargetDir@/@MaintenanceToolName@.exe", "@StartMenuDir@/Uninstall.lnk");
			else {
				component.addOperation("CreateShortcut", "@TargetDir@/@MaintenanceToolName@.exe", "@StartMenuDir@/@MaintenanceToolName@.lnk");
				component.addOperation("CreateShortcut", "@TargetDir@/@MaintenanceToolName@.exe", "@StartMenuDir@/ManagePackages.lnk", "--manage-packages");
				component.addOperation("CreateShortcut", "@TargetDir@/@MaintenanceToolName@.exe", "@StartMenuDir@/Update.lnk", "--updater");
				component.addOperation("CreateShortcut", "@TargetDir@/@MaintenanceToolName@.exe", "@StartMenuDir@/Uninstall.lnk", "uninstallOnly=1");
			}

			//... and desktop shortcut (if requested)
			var pageWidget = gui.pageWidgetByObjectName("DynamicShortcutPage");
			if (pageWidget !== null && pageWidget.shortcutCheckBox.checked)
				component.addOperation("CreateShortcut", "@TargetDir@/@ProductName@.exe", "@DesktopDir@/@Name@.lnk");
		} else if (installer.value("os") === "x11") {
			//x11 -> create .desktop file
			component.addOperation("CreateDesktopEntry",
								   "@BinaryName@.desktop",
								   "Version=1.1\nType=Application\nTerminal=false\nExec=@RunProgram@\nName=@Name@\nIcon=@TargetDir@/main.png");
		}
	} catch (e) {
		print(e);
	}
}
