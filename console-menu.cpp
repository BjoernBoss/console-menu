#include "console-menu.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#define NOMINMAX
#include <Windows.h>

/* implement the instance interface */
menu::Instance::Instance() : pHost(0) {}
menu::Host* menu::Instance::host() {
	return pHost;
}
void menu::Instance::init() {}
void menu::Instance::teardown() {}

/* implement the layout builder object */
menu::Layout::Entry::Entry() : id(0) {}
menu::Layout::Entry::Entry(EntryId id, const std::string& text) : id(id), text(text) {}
menu::Layout::Layout(bool updateLoop, const std::string& header) : updateLoop(updateLoop), header(header) {}
bool menu::Layout::add(EntryId id, const std::string& text) {
	/* check if the corresponding id is invalid or has already been defined */
	if (id == EntryIdInvalid || std::find_if(pEntries.begin(), pEntries.end(), [id](const Entry& ent) { return ent.id == id; }) != pEntries.end())
		return false;

	/* add the entry to the array */
	pEntries.emplace_back(id, text);
	return true;
}

/* implement the page behavior object */
menu::Behavior::Behavior() : traverse(Traverse::stay) {}
menu::Behavior::Behavior(const std::string& response, Traverse traverse, const std::string& target) : response(response), target(target), traverse(traverse) {}

/* implement the page interface */
menu::Page::Page(const char* identifier, const char* title) : pHost(0), pInstance(0), pIdentifier(identifier), pTitle(title) {}
menu::Host* menu::Page::host() {
	return pHost;
}
menu::Instance* menu::Page::instance() {
	return pInstance;
}
void menu::Page::init() { }
void menu::Page::teardown() { }
void menu::Page::load() {}
void menu::Page::unload() {}
bool menu::Page::update() {
	return false;
}

/* implement the host object */
menu::Host::Host(bool useCtrl) : pUseCtrl(useCtrl) {}
void menu::Host::fPrintMenu(const menu::Page* page, const menu::Layout& layout, const std::string& response) {
	/* clear the console screen */
	CONSOLE_SCREEN_BUFFER_INFO s = { 0 };
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(console, &s);
	DWORD written = 0;
	FillConsoleOutputCharacterA(console, ' ', s.dwSize.X * s.dwSize.Y, { 0, 0 }, &written);
	FillConsoleOutputAttribute(console, s.wAttributes, s.dwSize.X * s.dwSize.Y, { 0, 0 }, &written);
	SetConsoleCursorPosition(console, { 0, 0 });

	/* define the header bounds */
	std::string_view bound = "+--------------------------------------------------------------+";

	/* print the top bound, which includes the title */
	std::string topBound(bound);
	size_t titleLength = std::min((size_t)24, std::strlen(page->pTitle));
	size_t titleOffset = (bound.size() - titleLength) / 2;
	topBound[titleOffset - 1] = ' ';
	*std::copy(page->pTitle, page->pTitle + titleLength, topBound.begin() + titleOffset) = ' ';
	std::cout << topBound << std::endl;

	/* print the layout header */
	if (!layout.header.empty()) {
		std::stringstream lines(layout.header);

		/* iterate through the header and print it row by row and indent it */
		std::string line;
		while (std::getline(lines, line))
			std::cout << " " << line << std::endl;
		std::cout << bound << std::endl;
	}

	/* print all of the options */
	if (!layout.pEntries.empty()) {
		for (size_t i = 0; i < layout.pEntries.size(); i++) {
			std::cout << "  [" << (i < 10 && layout.pEntries.size() >= 10 ? "0" : "") << i << "] - ";
			std::cout << layout.pEntries[i].text << std::endl;
		}
		std::cout << bound << std::endl;
	}

	/* print the last response line by line and indent it */
	std::cout << std::endl;
	if (!response.empty()) {
		std::stringstream lines(response);
		std::string line;
		while (std::getline(lines, line))
			std::cout << " " << line << std::endl;
		std::cout << std::endl;
	}
}
void menu::Host::fTeardown(menu::Instance* instance) {
	/* unload all of the currently loaded pages */
	for (size_t i = pStack.size(); i > 0; i--)
		pPages[pStack[i - 1]]->unload();
	pStack.clear();

	/* teardown all of the pages */
	for (size_t i = 0; i < pPages.size(); i++)
		pPages[i]->teardown();
	pPages.clear();

	/* teardown the instance */
	instance->teardown();
}
menu::Host::Result menu::Host::fGetCTRLNumber(const std::string& text, bool peek, bool print, uint64_t& value) {
	static constexpr uint8_t KeyMap[12] = { VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9, VK_ADD, VK_SUBTRACT };
	value = 0;

	/* flush the input to remove any accidentally pressed keys */
	fFlushInput();

	/* notify the user about the requested input */
	if (!peek || print)
		std::cout << text << " (CTRL + NUMPAD; abort: '+'; backspace: '-'): ";
	uint16_t length = 0;

	/* loop until a number has been entered or control has been released, if a number is only optional */
	do {
		/* check if number can currently be entered (CTRL is down) */
		if (!(GetAsyncKeyState(VK_CONTROL) & 0x8000))
			continue;

		/* cache the current key-state */
		bool key[12] = { 0 };
		for (uint8_t i = 0; i < 12; i++)
			key[i] = GetAsyncKeyState(KeyMap[i]) & 0x8000;

		/* iterate through the keys as long as control is held */
		do {
			/* iterate through the keys and check if they have been pressed */
			for (uint8_t i = 0; i < 12; i++) {
				/* check the current key-state */
				if ((GetAsyncKeyState(KeyMap[i]) & 0x8000) == 0x0000) {
					key[i] = false;
					continue;
				}
				else if (key[i])
					continue;
				key[i] = true;

				/* process the affects of the key and apply them to the current number */
				if (i < 10) {
					std::cout << (uint8_t)('0' + i) << std::flush;
					value = (value * 10) + i;
					length++;
					continue;
				}

				/* check if only the last digit should be removed */
				if (i == 11) {
					if (length > 0) {
						/* reset the cursor */
						std::cout << "\b \b" << std::flush;
						value /= 10;
						length--;
					}
					continue;
				}

				/* clear all digits from the output */
				for (size_t i = 0; i < length; i++)
					std::cout << "\b \b";
				std::cout << std::flush;

				/* clear the entire number */
				length = 0;
				value = 0;

				/* wait for control to be released */
				std::cout << "aborted" << std::endl;
				while (GetAsyncKeyState(VK_CONTROL) & 0x8000)
					Sleep(1);
				fFlushInput();
				return Result::aborted;
			}

			/* yield cpu time */
			Sleep(1);
		} while (GetAsyncKeyState(VK_CONTROL) & 0x8000);
	} while (!peek && length == 0);

	/* flush the input to remove any accidentally pressed keys */
	fFlushInput();

	/* break the current line as the cursor is on the same line as the numbers and check if a value has been entered */
	if (length > 0)
		std::cout << std::endl;
	return length > 0 ? Result::number : Result::empty;
}
menu::Host::Result menu::Host::fGetDefaultNumber(const std::string& text, bool hex, bool abortable, uint64_t& value) {
	/* loop until a valid number has been read */
	while (true) {
		/* notify the user about the requested input */
		std::cout << text << ((hex || abortable) ? " (" : ": ");
		if (hex)
			std::cout << "in hex";
		if (abortable)
			std::cout << (hex ? "; abort: '!'" : "abort: '!'");
		if (hex || abortable)
			std::cout << "): ";

		/* read all input from the std-in */
		std::string line;
		std::getline(std::cin, line);
		if (!line.empty()) {
			/* check if the user tried to abort */
			if (abortable && line.size() == 1 && line[0] == '!')
				return Result::aborted;

			/* parse the response */
			std::stringstream sstr(line);
			if ((sstr >> (hex ? std::hex : std::dec) >> value) && sstr.eof())
				return Result::number;
		}
		std::cout << "invalid input!" << std::endl;
	}
}
size_t menu::Host::fResolvePage(const char* identifier) const {
	for (size_t i = 0; i < pPages.size(); i++) {
		if (std::strcmp(pPages[i]->pIdentifier, identifier) == 0)
			return i;
	}
	return (size_t)-1;
}
void menu::Host::fFlushInput() const {
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
}
menu::Host* menu::Host::acquire(bool useCtrl) {
	return new Host(useCtrl);
}
void menu::Host::release() {
	delete this;
}
bool menu::Host::getUseCtrl() const {
	return pUseCtrl;
}
void menu::Host::setUseCtrl(bool useCtrl) {
	pUseCtrl = useCtrl;
}
void menu::Host::run(menu::Instance* instance) {
	/* initialize the instance */
	instance->pHost = this;
	instance->init();

	/* initialize all of the pages */
	for (menu::Page* page : pPages) {
		page->pHost = this;
		page->pInstance = instance;
		page->init();
	}

	/* lookup the root-page */
	const char* rootIdentifier = instance->root();
	size_t root = fResolvePage(rootIdentifier);

	/* check if a root has been found */
	if (root == (size_t)-1) {
		fTeardown(instance);
		std::cout << "root page <" << rootIdentifier << "> not found!" << std::endl;
		return;
	}

	/* load the root page */
	pStack.push_back(root);
	pPages[root]->load();

	/* enter the primary menu loop */
	std::string response;
	while (true) {
		menu::Page* top = pPages[pStack.back()];
		menu::EntryId selected = menu::EntryIdInvalid;

		/* loop until valid input has been entered by the user */
		do {
			/* query the layout for the current menu and print it */
			menu::Layout layout = std::move(top->construct());
			fPrintMenu(top, layout, response);

			/* read the input from the user */
			if (!layout.updateLoop) {
				uint64_t input = 0;
				while ((pUseCtrl ? fGetCTRLNumber("select an option", false, false, input) : fGetDefaultNumber("select an option", false, false, input)) != Result::number) {}
				if (input < layout.pEntries.size())
					selected = layout.pEntries[(size_t)input].id;
				else
					response = "index out of range!";
			}

			/* enter the update loop until the page interrupts it or the user inserts input */
			else {
				bool wasPrinted = false;
				do {
					/* check if input has been entered by the user */
					if (layout.pEntries.empty())
						continue;
					uint64_t input = 0;
					Result result = fGetCTRLNumber("select an option", true, !wasPrinted, input);
					wasPrinted = true;
					if (result != Result::number)
						continue;

					/* validate the input */
					if (input < layout.pEntries.size())
						selected = layout.pEntries[(size_t)input].id;
					else
						response = "index out of range!";
					break;
				} while (top->update());
			}
		} while (selected == menu::EntryIdInvalid);

		/* evaluate the response */
		menu::Behavior behavior = top->evaluate(selected);
		response = behavior.response;

		/* resolve the optional page */
		size_t target = fResolvePage(behavior.target.c_str());

		/* handle the traversal-request */
		if (behavior.traverse == menu::Traverse::move || behavior.traverse == menu::Traverse::push) {
			/* check if the target page is valid */
			if (target == (size_t)-1) {
				fTeardown(instance);
				std::cout << "target page <" << behavior.target << "> not found!" << std::endl;
				return;
			}

			/* check if the page is already loaded */
			for (size_t i = 0; i < pStack.size(); i++) {
				if (pStack[i] != target)
					continue;
				fTeardown(instance);
				std::cout << "target page <" << behavior.target << "> already loaded!" << std::endl;
				return;
			}

			/* unload the current page and replace it by the new page */
			if (behavior.traverse == menu::Traverse::move) {
				top->unload();
				pStack.back() = target;
			}

			/* push the new page */
			else
				pStack.push_back(target);

			/* load the new page */
			pPages[target]->load();
		}
		else if (behavior.traverse == menu::Traverse::pop) {
			/* unload the current page */
			top->unload();
			pStack.pop_back();
		}
		else if (behavior.traverse == menu::Traverse::dest) {
			/* unload the pages until the the stack is empty or the target has been reached */
			do {
				pPages[pStack.back()]->unload();
				pStack.pop_back();
			} while (pStack.size() > 0 && pStack.back() != target);
		}
		else if (behavior.traverse == menu::Traverse::root) {
			/* unload the pages until the root has been reached */
			do {
				pPages[pStack.back()]->unload();
				pStack.pop_back();
			} while (pStack.size() > 1);
		}
		else if (behavior.traverse == menu::Traverse::exit) {
			/* unload all resources and exit the menu */
			fTeardown(instance);
			std::cout << "menu successfully closed!" << std::endl;
			return;
		}

		/* check if the stack is empty */
		if (pStack.empty()) {
			fTeardown(instance);
			std::cout << "returning from last page!" << std::endl;
			return;
		}
	}
}
bool menu::Host::add(menu::Page* page) {
	/* check if a page with the corresponding identifier already exists */
	if (fResolvePage(page->pIdentifier) != (size_t)-1)
		return false;

	/* add the page to the set of pages */
	pPages.push_back(page);
	return true;
}
bool menu::Host::isLoaded(const char* identifier) const {
	/* check if a page with the corresponding identifier is loaded */
	for (size_t i = 0; i < pStack.size(); i++) {
		if (std::strcmp(pPages[pStack[i]]->pIdentifier, identifier) == 0)
			return true;
	}
	return false;
}
bool menu::Host::isAdded(const char* identifier) const {
	return fResolvePage(identifier) != (size_t)-1;
}
bool menu::Host::getNumber(const std::string& text, uint64_t& value, bool hex) {
	/* check if the control-input method should be used */
	if (pUseCtrl && !hex)
		return fGetCTRLNumber(text, false, false, value) == Result::number;
	return fGetDefaultNumber(text, hex, true, value) == Result::number;
}