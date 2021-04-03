#include "ConsoleMenu.h"
using namespace std;

//implementation of the constructor-function - menupage
MenuPage::MenuPage()
{

}
MenuPage::~MenuPage()
{

}

//implementation of the private-functions - menu
void MenuStruct::printMenu(std::string title, MenuPageMenu* page, std::string msg)
{
	//clear the screen
	CONSOLE_SCREEN_BUFFER_INFO s;
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(console, &s);
	DWORD written = 0;
	FillConsoleOutputCharacterA(console, ' ', s.dwSize.X * s.dwSize.Y, { 0, 0 }, &written);
	FillConsoleOutputAttribute(console, s.wAttributes, s.dwSize.X * s.dwSize.Y, { 0, 0 }, &written);
	SetConsoleCursorPosition(console, { 0, 0 });

	//print the header
	if (title.size() > 24)
		title.resize(24);
	title = " " + title + " ";
	uint64_t totalLineCount = 62 - title.size();
	for (uint64_t i = 0; i < totalLineCount; i++)
	{
		if (i < (totalLineCount >> 1) + (totalLineCount & 0x01))
			title = "-" + title;
		else
			title = title + "-";
	}
	title = "+" + title + "+";
	cout << title << endl;

	//print the string
	if (page->MenuString.size() > 0)
	{
		//print the separate rows
		uint64_t index = 0;
		while (true)
		{
			uint64_t newIndex = page->MenuString.find('\n', index);
			if (newIndex == string::npos)
			{
				cout << "  " << page->MenuString.substr(index) << endl;
				break;
			}
			cout << "  " << page->MenuString.substr(index, newIndex - index) << endl;
			index = newIndex + 1;
			if (page->MenuString.size() > index)
			{
				if (page->MenuString.at(index) == '\r')
					index++;
			}
			else
				break;
		}
		cout << "+--------------------------------------------------------------+" << endl;
	}

	//print the menu
	if (page->entries.size() > 0)
	{
		for (uint64_t i = 0; i < page->entries.size(); i++)
		{
			if (page->entries.size() >= 10 && i < 10)
				cout << "  [0" << i << "] - ";
			else
				cout << "  [" << i << "] - ";
			cout << page->entries.at(i).text << endl;
		}
		cout << "+--------------------------------------------------------------+" << endl;
	}

	//print the last message
	cout << endl;
	if (msg.size() > 0)
		cout << msg << endl << endl;
}
void MenuStruct::unloadStack()
{
	//unload all of the menus
	for (uint64_t i = loadedStack.size(); i > 0; i--)
		if(menuPages.at(loadedStack.at(i - 1)).unload != 0)
			menuPages.at(loadedStack.at(i - 1)).unload(this);
	loadedStack.clear();

	//unload the global-data
	if (globalUnload != 0)
		globalUnload(this);
}
uint64_t MenuStruct::getCTRLNumber()
{
	//print the message
	cout << "select an option (CTRL + NUMPAD; cancel: '+'; backspace: '-'): ";

	//variables
	bool control_pressed = false;
	bool key_added = false;
	uint64_t value = 0;
	bool key_state[12];
	uint16_t charLen = 0;

	//read all of the keystates
	for (uint8_t i = 0; i < 12; i++)
		key_state[i] = (((i >= 10) ? GetAsyncKeyState(i == 10 ? VK_ADD : VK_SUBTRACT) : GetAsyncKeyState(VK_NUMPAD0 + i)) & 0x8000);

	//enter the loop
	do {
		//check if control has been pressed
		control_pressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000);

		//check if control is pressed
		if (control_pressed)
		{
			//loop through the keys
			for (uint8_t i = 0; i < 12; i++)
			{
				if (i >= 10) {
					if (GetAsyncKeyState(i == 10 ? VK_ADD : VK_SUBTRACT) & 0x8000)
					{
						if (!key_state[i])
						{
							if (i == 10) {
								std::cout << '+';
								charLen++;
								while (GetAsyncKeyState(VK_CONTROL) & 0x8000);
								while (charLen-- > 0)
									std::cout << "\b \b";
								charLen = 0;
								key_added = false;
								value = 0;
							}
							else if (charLen > 0) {
								std::cout << "\b \b";
								value /= 10;
								charLen--;
								if (charLen == 0)
									key_added = false;
							}
							key_state[i] = true;
						}
					}
					else
						key_state[i] = false;
				}
				else if (GetAsyncKeyState(VK_NUMPAD0 + i) & 0x8000)
				{
					if (!key_state[i])
					{
						std::cout << (uint8_t)('0' + i);
						value = (value * 10) + i;
						key_added = true;
						key_state[i] = true;
						charLen++;
					}
				}
				else
					key_state[i] = false;
			}
		}
		else
		{
			//read all of the keystates
			for (uint8_t i = 0; i < 12; i++)
				key_state[i] = (((i >= 10) ? GetAsyncKeyState(i == 10 ? VK_ADD : VK_SUBTRACT) : GetAsyncKeyState(VK_NUMPAD0 + i)) & 0x8000);
		}
		Sleep(1);
	} while (!key_added || control_pressed);
	cout << endl;
	return value;
}
uint64_t MenuStruct::getDefaultNumber()
{
	while (true) {
		//print the message
		cout << "select an option: ";

		//receive the input
		string str;
		getline(cin, str);
		
		//convert the input
		uint64_t result = 0;
		stringstream sstr(str);
		if (sstr.str().size() == 0) {
			cout << "invalid input!" << endl;
			continue;
		}
		sstr >> result;
		if (!sstr.eof()) {
			cout << "invalid input!" << endl;
			continue;
		}
		return result;
	}
}
bool MenuStruct::peekCTRLNumber(uint64_t* nbr)
{
	//variables
	bool control_pressed = false;
	*nbr = 0;
	bool key_added = false;
	bool key_state[12];
	uint16_t charLen = 0;

	//read all of the keystates
	for (uint8_t i = 0; i < 12; i++)
		key_state[i] = (((i >= 10) ? GetAsyncKeyState(i == 10 ? VK_ADD : VK_SUBTRACT) : GetAsyncKeyState(VK_NUMPAD0 + i)) & 0x8000);

	//enter the loop
	do {
		//check if control has been pressed
		control_pressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000);

		//check if control is pressed
		if (control_pressed)
		{
			//loop through the keys
			for (uint8_t i = 0; i < 12; i++)
			{
				if (i >= 10) {
					if (GetAsyncKeyState(i == 10 ? VK_ADD : VK_SUBTRACT) & 0x8000)
					{
						if (!key_state[i])
						{
							if (i == 10) {
								std::cout << '+';
								charLen++;
								while (GetAsyncKeyState(VK_CONTROL) & 0x8000);
								while(charLen-- > 0)
									std::cout << "\b \b";
								charLen = 0;
								return false;
							}
							else if (charLen > 0) {
								std::cout << "\b \b";
								*nbr /= 10;
								charLen--;
								if (charLen == 0)
									key_added = false;
							}
							key_state[i] = true;
						}
					}
					else
						key_state[i] = false;
				}
				else if (GetAsyncKeyState(VK_NUMPAD0 + i) & 0x8000)
				{
					if (!key_state[i])
					{
						std::cout << (uint8_t)('0' + i);
						*nbr = (*nbr * 10) + i;
						key_added = true;
						key_state[i] = true;
						charLen++;
					}
				}
				else
					key_state[i] = false;
			}
		}
		else
		{
			//read all of the keystates
			for (uint8_t i = 0; i < 12; i++)
				key_state[i] = (((i >= 10) ? GetAsyncKeyState(i == 10 ? VK_ADD : VK_SUBTRACT) : GetAsyncKeyState(VK_NUMPAD0 + i)) & 0x8000);
		}
		Sleep(1);
	} while (control_pressed);
	if(key_added)
		cout << endl;
	return key_added;
}

//implementation of the object-functions - menu
MenuStruct* MenuStruct::acquire(bool useCtrl)
{
	//allocate the new object
	MenuStruct* newMenu = (MenuStruct*)malloc(sizeof(MenuStruct));
	if (newMenu == 0)
		return 0;
	memset(newMenu, 0, sizeof(MenuStruct));

	//initialize the resources
	newMenu->loadedStack = std::vector<uint64_t>();
	newMenu->menuPages = std::vector<MenuPage>();
	newMenu->localData = std::vector<void*>();
	newMenu->useCTRL = useCtrl;
	return newMenu;
}
MenuStruct* MenuStruct::acquire(bool useCtrl, void(*globLoad)(MenuStruct* menuStruct))
{
	//allocate the new object
	MenuStruct* newMenu = (MenuStruct*)malloc(sizeof(MenuStruct));
	if (newMenu == 0)
		return 0;
	memset(newMenu, 0, sizeof(MenuStruct));

	//initialize the resources
	newMenu->globalLoad = globLoad;
	newMenu->loadedStack = std::vector<uint64_t>();
	newMenu->menuPages = std::vector<MenuPage>();
	newMenu->localData = std::vector<void*>();
	newMenu->useCTRL = useCtrl;
	return newMenu;
}
void MenuStruct::release()
{
	//free all of the resources
	menuPages.~vector();
	loadedStack.~vector();
	localData.~vector();

	//free this object
	free(this);
}

//implementation of the public functions - menu
bool MenuStruct::getUseCtrl() 
{
	return useCTRL;
}
void MenuStruct::setUseCtrl(bool useCtrl) 
{
	useCTRL = useCtrl;
}
void MenuStruct::run(uint64_t rootId)
{
	//load the global-data
	if (globalLoad != 0)
		globalLoad(this);

	//find the root-menu
	currentMenu = (uint64_t)-1;
	for (uint64_t i = 0; i < menuPages.size(); i++)
	{
		if (menuPages.at(i).id == rootId)
		{
			currentMenu = i;
			break;
		}
	}
	if (currentMenu == (uint64_t)-1)
	{
		unloadStack();
		cout << hex << "root-menu not found <" << rootId << "> !" << endl;
		return;
	}

	//load the menu
	loadedStack.push_back(currentMenu);
	if (menuPages.at(currentMenu).load != 0)
		menuPages.at(currentMenu).load(this);

	//enter the main-loop
	string message = "";
	while (true)
	{
		//render the menu
		bool input_valid = false;
		uint64_t input_nbr = 0;
		MenuPageMenu pageMenu;
		do {
			//query the content of the current menu
			flushConsole();
			if (menuPages.at(currentMenu).menu == 0) {
				unloadStack();
				cout << hex << "function-menu not found <" << menuPages.at(currentMenu).id << "> !" << endl;
				return;
			}
			pageMenu = menuPages.at(currentMenu).menu(this);
			
			//print the menu
			printMenu(menuPages.at(currentMenu).title, &pageMenu, message);
			
			//handle the input-request
			if (!pageMenu.update || menuPages.at(currentMenu).update == 0) {
				input_nbr = useCTRL ? getCTRLNumber() : getDefaultNumber();
				if (input_nbr < pageMenu.entries.size())
					input_valid = true;
				else
					message = "index out of range!";
			}
			else {
				//enter the wait-loop
				cout << "select an option (CTRL + NUMPAD; cancel: '+'; backspace: '-'): ";
				do {
					if (peekCTRLNumber(&input_nbr))
					{
						if (input_nbr < pageMenu.entries.size())
						{
							input_valid = true;
							break;
						}
						else
						{
							message = "index out of range!";
							input_valid = false;
							break;
						}
					}
					flushConsole();
				} while (menuPages.at(currentMenu).update(this));
			}
		} while (!input_valid);
		message.clear();

		//evaluate the input
		flushConsole();
		if (menuPages.at(currentMenu).eval == 0) {
			unloadStack();
			cout << hex << "function-eval not found <" << menuPages.at(currentMenu).id << "> !" << endl;
			return;
		}
		MenuPageEval pageEval = menuPages.at(currentMenu).eval(this, pageMenu.entries.at(input_nbr).type);
		message = pageEval.message;

		//handle the evaluated request
		if (pageEval.traverse == MenuTraverse::move)
		{
			//resolve the new menu
			uint64_t newMenu = (uint64_t)-1;
			for (uint64_t i = 0; i < menuPages.size(); i++)
			{
				if (menuPages.at(i).id == pageEval.menuId)
				{
					newMenu = i;
					break;
				}
			}
			if (newMenu == (uint64_t)-1)
			{
				unloadStack();
				cout << hex << "new-menu not found <" << pageEval.menuId << "> !" << endl;
				return;
			}

			//check if the menu is already loaded
			for (uint64_t i = 0; i < loadedStack.size(); i++)
			{
				if (loadedStack.at(i) == newMenu)
				{
					unloadStack();
					cout << hex << "new-menu already loaded <" << pageEval.menuId << "> !" << endl;
					return;
				}
			}

			//unload the current-menu
			if (menuPages.at(currentMenu).unload != 0)
				menuPages.at(currentMenu).unload(this);
			loadedStack.pop_back();

			//load the new menu
			currentMenu = newMenu;
			loadedStack.push_back(currentMenu);
			if (menuPages.at(currentMenu).load != 0)
				menuPages.at(currentMenu).load(this);
		}
		else if (pageEval.traverse == MenuTraverse::pop)
		{
			//unload the current-menu
			if (menuPages.at(currentMenu).unload != 0)
				menuPages.at(currentMenu).unload(this);
			loadedStack.pop_back();

			//check if the stack is empty
			if (loadedStack.size() == 0)
			{
				unloadStack();
				cout << "returning from last menu!" << endl;
				return;
			}

			//update the current menu
			currentMenu = loadedStack.at(loadedStack.size() - 1);
		}
		else if (pageEval.traverse == MenuTraverse::dest)
		{
			//unload the menus until the root has been hit or the id
			do {
				if (menuPages.at(currentMenu).unload != 0)
					menuPages.at(currentMenu).unload(this);
				loadedStack.pop_back();
				if (loadedStack.size() == 0)
				{
					unloadStack();
					cout << "returning from last menu!" << endl;
					return;
				}
				currentMenu = loadedStack.at(loadedStack.size() - 1);
			} while (menuPages.at(currentMenu).id != pageEval.menuId);
		}
		else if (pageEval.traverse == MenuTraverse::root)
		{
			//unload the menus until the root has been hit or the id
			do {
				if (menuPages.at(currentMenu).unload != 0)
					menuPages.at(currentMenu).unload(this);
				loadedStack.pop_back();
				if (loadedStack.size() == 0)
				{
					unloadStack();
					cout << "returning from last menu!" << endl;
					return;
				}
				currentMenu = loadedStack.at(loadedStack.size() - 1);
			} while (loadedStack.size() > 1);
		}
		else if (pageEval.traverse == MenuTraverse::push)
		{
			//resolve the new menu
			uint64_t newMenu = (uint64_t)-1;
			for (uint64_t i = 0; i < menuPages.size(); i++)
			{
				if (menuPages.at(i).id == pageEval.menuId)
				{
					newMenu = i;
					break;
				}
			}
			if (newMenu == (uint64_t)-1)
			{
				unloadStack();
				cout << hex << "new-menu not found <" << pageEval.menuId << "> !" << endl;
				return;
			}

			//check if the menu is already loaded
			for (uint64_t i = 0; i < loadedStack.size(); i++)
			{
				if (loadedStack.at(i) == newMenu)
				{
					unloadStack();
					cout << hex << "new-menu already loaded <" << pageEval.menuId << "> !" << endl;
					return;
				}
			}

			//load the new menu
			currentMenu = newMenu;
			loadedStack.push_back(currentMenu);
			if (menuPages.at(currentMenu).load != 0)
				menuPages.at(currentMenu).load(this);
		}
		else if (pageEval.traverse == MenuTraverse::exit) {
			//unload the resources
			unloadStack();
			cout << "menu successfully closed!" << endl;
			return;
		}
	}
}
bool MenuStruct::attach(MenuPage menu)
{
	//check if a menu with this id already exists
	for (uint64_t i = 0; i < menuPages.size(); i++)
		if (menuPages.at(i).id == menu.id)
			return false;

	//add the menu
	menuPages.push_back(menu);
	localData.push_back(0);
	return true;
}
bool MenuStruct::detach(uint64_t menuId)
{
	//find the menu
	for (uint64_t i = 0; i < menuPages.size(); i++) {
		if (menuPages.at(i).id == menuId) {
			//check if the menu is loaded
			for (uint64_t j = 0; j < loadedStack.size(); j++)
				if (loadedStack.at(j) == i)
					return false;

			//remove the page
			menuPages.erase(menuPages.begin() + i);
			localData.erase(localData.begin() + i);
			return true;
		}
	}
	return true;
}
void MenuStruct::setGlobalLoad(void(*func)(MenuStruct* menuStruct))
{
	globalLoad = func;
}
void MenuStruct::setGlobalUnload(void(*func)(MenuStruct* menuStruct))
{
	globalUnload = func;
}
bool MenuStruct::isLoaded(uint64_t id)
{
	for (uint64_t i = 0; i < loadedStack.size(); i++)
	{
		if (menuPages.at(loadedStack.at(i)).id == id)
			return true;
	}
	return false;
}
void* MenuStruct::getGlobalData()
{
	return globalData;
}
void MenuStruct::setGlobalData(void* ptr)
{
	globalData = ptr;
}
void* MenuStruct::getLocalData(uint64_t menuId)
{
	for (uint64_t i = 0; i < menuPages.size(); i++)
	{
		if (menuPages.at(i).id == menuId)
			return localData.at(i);
	}
	return 0;
}
void MenuStruct::setLocalData(uint64_t menuId, void* ptr)
{
	for (uint64_t i = 0; i < menuPages.size(); i++)
	{
		if (menuPages.at(i).id == menuId)
		{
			localData.at(i) = ptr;
			return;
		}
	}
}
bool MenuStruct::getNumber(std::string preString, uint64_t* index)
{
	//check if default console-input is requested
	if (!useCTRL) {
		//receive the input
		cout << preString << ": ";
		string str;
		getline(cin, str);

		//convert the input
		uint64_t result = 0;
		stringstream sstr(str);
		if (sstr.str().size() == 0)
			return false;
		sstr >> result;
		if (!sstr.eof())
			return false;
		*index = result;
		return true;
	}

	//print the message
	cout << preString << " (CTRL + NUMPAD; cancel: '+'; backspace: '-'): ";
	
	//variables
	bool control_pressed = false;
	bool key_added = false;
	uint64_t value = 0;
	bool key_state[12];
	uint16_t charLen = 0;

	//read all of the keystates
	for (uint8_t i = 0; i < 12; i++)
		key_state[i] = (((i >= 10) ? GetAsyncKeyState(i == 10 ? VK_ADD : VK_SUBTRACT) : GetAsyncKeyState(VK_NUMPAD0 + i)) & 0x8000);

	//enter the loop
	do {
		//check if control has been pressed
		control_pressed = (GetAsyncKeyState(VK_CONTROL) & 0x8000);

		//check if control is pressed
		if (control_pressed)
		{
			//loop through the keys
			for (uint8_t i = 0; i < 12; i++)
			{
				if (i >= 10) {
					if (GetAsyncKeyState(i == 10 ? VK_ADD : VK_SUBTRACT) & 0x8000)
					{
						if (!key_state[i])
						{
							if (i == 10) {
								std::cout << '+';
								while (GetAsyncKeyState(VK_CONTROL) & 0x8000);
								return false;
							}
							else if(charLen > 0) {
								std::cout << "\b \b";
								value /= 10;
								charLen--;
								if (charLen == 0)
									key_added = false;
							}
							key_state[i] = true;
						}
					}
					else
						key_state[i] = false;
				}
				else if (GetAsyncKeyState(VK_NUMPAD0 + i) & 0x8000)
				{
					if (!key_state[i])
					{
						std::cout << (uint8_t)('0' + i);
						value = (value * 10) + i;
						key_added = true;
						key_state[i] = true;
						charLen++;
					}
				}
				else
					key_state[i] = false;
			}
		}
		else
		{
			//read all of the keystates
			for (uint8_t i = 0; i < 12; i++)
				key_state[i] = (((i >= 10) ? GetAsyncKeyState(i == 10 ? VK_ADD : VK_SUBTRACT) : GetAsyncKeyState(VK_NUMPAD0 + i)) & 0x8000);
		}
		Sleep(1);
	} while (!key_added || control_pressed);
	*index = value;
	cout << endl;
	return true;
}
bool MenuStruct::getNumberHex(std::string preString, uint64_t* index)
{
	//print the message
	cout << preString << " (in hex): ";

	*index = 0;
	while (true) {
		std::string str;
		flushConsole();
		getline(std::cin, str);
		if (str.size() > 0)
		{
			//receive the input
			std::stringstream sstr(str);
			sstr >> std::hex >> *index;
			if (!sstr.eof())
				return false;
			else
				return true;
		}
	}
}
void MenuStruct::flushConsole()
{
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
}