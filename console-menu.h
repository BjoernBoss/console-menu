#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <inttypes.h>
#include <iostream>
#include <sstream>

//predefine the class
class MenuStruct;

//create the menupage-class
enum class MenuTraverse : uint8_t
{
	stay,
	move,
	push, 
	pop, 
	dest,
	root,
	exit
};
struct MenuEntry
{
	uint64_t type;
	std::string text;
	MenuEntry()
	{
		type = 0;
		text = "";
	}
	MenuEntry(uint64_t tp, std::string txt)
	{
		type = tp;
		text = txt;
	}
};
struct MenuPageMenu
{
	bool update;
	std::vector<MenuEntry> entries;
	std::string MenuString;
	MenuPageMenu()
	{
		update = false;
		entries = std::vector<MenuEntry>();
		MenuString = "";
	}
	MenuPageMenu(bool updateLoop, std::string String)
	{
		update = updateLoop;
		entries = std::vector<MenuEntry>();
		MenuString = String;
	}
};
struct MenuPageEval
{
	std::string message;
	MenuTraverse traverse;
	uint64_t menuId;
	MenuPageEval()
	{
		traverse = MenuTraverse::stay;
		menuId = 0;
		message = "";
	}
	MenuPageEval(std::string msg, MenuTraverse trav, uint64_t id)
	{
		message = msg;
		traverse = trav;
		menuId = id;
	}
};
struct MenuPage
{
	//implement the constructors
	MenuPage();
	~MenuPage();
	
	//define the attributes used to describe the menu
	uint64_t id;
	const char* title;

	//define the functions
	void(*load)(MenuStruct* menuStruct);
	void(*unload)(MenuStruct* menuStruct);
	bool(*update)(MenuStruct* menuStruct);
	MenuPageMenu(*menu)(MenuStruct* menuStruct);
	MenuPageEval(*eval)(MenuStruct* menuStruct, uint64_t type);
};

//create the menustruct-class
class MenuStruct
{
private:
	std::vector<uint64_t> loadedStack;
	std::vector<MenuPage> menuPages;
	std::vector<void*> localData;
	uint64_t currentMenu;
	void* globalData;
	void(*globalLoad)(MenuStruct* menuStruct);
	void(*globalUnload)(MenuStruct* menuStruct);
	bool useCTRL;
private:
	void printMenu(std::string title, MenuPageMenu* page, std::string msg);
	void unloadStack();
	uint64_t getCTRLNumber();
	uint64_t getDefaultNumber();
	bool peekCTRLNumber(uint64_t* nbr);
public:
	MenuStruct() = delete;
	MenuStruct(const MenuStruct& ms) = delete;
	MenuStruct(const MenuStruct&& ms) = delete;
	MenuStruct(MenuStruct* ms) = delete;
	~MenuStruct() = delete;
public:
	static MenuStruct* acquire(bool useCtrl);
	static MenuStruct* acquire(bool useCtrl, void(*globLoad)(MenuStruct* menuStruct));
	void release();
public:
	bool getUseCtrl();
	void setUseCtrl(bool useCtrl);
	void run(uint64_t rootId);
	bool attach(MenuPage menu);
	bool detach(uint64_t menuId);
	void setGlobalLoad(void(*func)(MenuStruct* menuStruct));
	void setGlobalUnload(void(*func)(MenuStruct* menuStruct));
	bool isLoaded(uint64_t id);
	void* getGlobalData();
	void setGlobalData(void* ptr);
	void* getLocalData(uint64_t menuId);
	void setLocalData(uint64_t menuId, void* ptr);
	bool getNumber(std::string preString, uint64_t* index);
	bool getNumberHex(std::string preString, uint64_t* index);
	void flushConsole();
};