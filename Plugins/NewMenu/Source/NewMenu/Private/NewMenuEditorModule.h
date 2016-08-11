#pragma once

#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "NewMenuEditor"

class NewMenuEditorModule : public IModuleInterface
{

private:

	TSharedPtr<FExtender> MainMenuExtender;

public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void CreateNewMenu();
	static void OnCreateNewMenu(FMenuBarBuilder& InMenuBarBuilder);

	static void OnCreateNewMenu2(FMenuBarBuilder& InMenuBarBuilder);

	static void createThumbnailMaterialInstance(FMenuBuilder& InMenuBarBuilder);

	static void colorChangeMaterial(FMenuBuilder& InMenuBarBuilder);
};