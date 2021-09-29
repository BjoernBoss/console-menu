/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2021 Bjoern Boss Henrichsen */
#pragma once

#include <inttypes.h>
#include <string>
#include <vector>

namespace menu {
	class Host;

	/* define the types and enums */
	using EntryId = size_t;
	static constexpr EntryId EntryIdInvalid = (EntryId)-1;
	enum class Traverse : uint8_t {
		stay,
		move,
		push,
		pop,
		dest,
		root,
		exit
	};

	/* define the instance interface */
	class Instance {
		friend class Host;
	private:
		Host* pHost;

	protected:
		Instance();
		virtual ~Instance() = default;
		Instance(Instance&&) = delete;
		Instance(const Instance&) = delete;

	public:
		Host* host();
		virtual void init();
		virtual void teardown();
		virtual const char* root() = 0;
	};

	/* define the layout builder object */
	class Layout {
		friend class Host;
	private:
		struct Entry {
		public:
			EntryId id;
			std::string text;

		public:
			Entry();
			Entry(EntryId id, const std::string& text);
		};

	private:
		std::vector<Entry> pEntries;

	public:
		bool updateLoop;
		std::string header;

	public:
		Layout(bool updateLoop, const std::string& header);
		~Layout() = default;
		Layout(Layout&&) = default;
		Layout(const Layout&) = default;

	public:
		bool add(EntryId id, const std::string& text);
	};

	/* define the page behavior object */
	class Behavior {
		friend class Host;
	public:
		std::string response;
		std::string target;
		Traverse traverse;

	public:
		Behavior();
		Behavior(const std::string& response, Traverse traverse, const std::string& target = "");
		~Behavior() = default;
		Behavior(Behavior&&) = default;
		Behavior(const Behavior&) = default;
	};

	/* define the page interface */
	class Page {
		friend class Host;
	private:
		Host* pHost;
		Instance* pInstance;
		const char* pIdentifier;
		const char* pTitle;

	protected:
		Page(const char* identifier, const char* title);
		virtual ~Page() = default;
		Page(Page&&) = delete;
		Page(const Page&) = delete;

	public:
		Host* host();
		Instance* instance();
		virtual void init();
		virtual void teardown();
		virtual void load();
		virtual void unload();
		virtual bool update();
		virtual Layout construct() = 0;
		virtual Behavior evaluate(EntryId id) = 0;
	};

	/* define the host object */
	class Host {
	private:
		enum class Result : uint8_t {
			number,
			aborted,
			empty
		};

	private:
		std::vector<size_t> pStack;
		std::vector<menu::Page*> pPages;
		bool pUseCtrl;

	private:
		Host(bool useCtrl);
		Host(Host&&) = delete;
		Host(const Host&) = delete;
		~Host() = default;

	private:
		void fPrintMenu(const menu::Page* page, const menu::Layout& layout, const std::string& response);
		void fTeardown(menu::Instance* instance);
		Result fGetCTRLNumber(const std::string& text, bool peek, bool print, uint64_t& value);
		Result fGetDefaultNumber(const std::string& text, bool hex, bool abortable, uint64_t& value);
		size_t fResolvePage(const char* identifier) const;
		void fFlushInput() const;

	public:
		static Host* acquire(bool useCtrl);
		void release();

	public:
		bool getUseCtrl() const;
		void setUseCtrl(bool useCtrl);

		void run(menu::Instance* instance);
		bool add(menu::Page* page);
		bool isLoaded(const char* identifier) const;
		bool isAdded(const char* identifier) const;

		bool getNumber(const std::string& text, uint64_t& value, bool hex);
	};
}