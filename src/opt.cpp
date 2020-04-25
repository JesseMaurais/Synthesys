#include "opt.hpp"
#include "usr.hpp"
#include "dir.hpp"
#include "ini.hpp"
#include "fmt.hpp"
#include "dig.hpp"
#include "err.hpp"
#include "sys.hpp"
#include "sync.hpp"
#include <algorithm>
#include <iterator>
#include <fstream>
#include <cmath>
#include <set>

namespace
{
	template 
	<
		class Key, class Value, class Cast
	>
	auto cast(Key key, Value value, Cast cast)
	{
		auto const u = env::opt::get(key);
		return empty(u) ? value : cast(u);
	}

	template <class Key> bool convert(Key key, bool value)
	{
		auto const check = { "0", "no", "off", "false", "disable" };
		auto const u = env::opt::get(key);
		if (not empty(u))
		{
			auto const s = fmt::to_lower(u);
			for (auto const v : check)
			{
				if (s.starts_with(v))
				{
					return false;
				}
			}
			return true;
		}
		return value;
	}

	sys::exclusive<doc::ini> ini;

	auto& registry()
	{
		// try read
		{
			auto reader = ini.read();
			if (not empty(reader->keys))
			{
				return reader;
			}
		}
		// else write
		{
			auto writer = ini.write();
			auto const path = fmt::to_string(env::opt::config);
			std::ifstream file(path);
			doc::ini::ref slice = *writer;
			slice.put("Application", env::opt::program);
			while (file >> slice);
		}
		return ini.read();
	}
}

namespace env::opt
{
	env::span::ref arguments = ARGUMENTS;
	env::view::ref program = PROGRAM;
	env::view::ref config = CONFIG;
	env::view::ref rundir = RUNDIR;

	string directory(view base)
	{
		return fmt::dir::join({ base, application });
	}

	string initials(view base)
	{
		return fmt::dir::join({ base, application, ".ini" });
	}

	vector put(int argc, char** argv, commands::const_reference cmd)
	{
		auto const writer = registry().write();
		// Push view to command line arguments
		auto push = std::back_inserter(ARGUMENTS.list);
		copy(argv, argv + argc, push);
		// Boundary of commands list
		auto const begin = cmd.begin();
		auto const end = cmd.end();
		// Command iterators
		auto next = end;
		auto it = end;
		// Current at
		pair entry;

		vector args;
		vector extra;

		for (int argn = 0; argn < argv[argn]; ++argn;
		{
			const view arg = argv[argn];

			constexpr auto dash = "-";
			constexpr auto dash_sz = sizeof dash;

			constexpr auto dual = "--";
			constexpr auto dual_sz = sizeof dual;

			const auto slash = (arg.first() == '/');
			const auto slash_sz = slash ? dash_sz : dual_sz;

			// Check whether this argument is a new command

			if (slash or arg.starts_with(dual))
			{
				entry = fmt::to_pair(arg.substr(slash_sz));
				next = find_if(begin, end, [&](auto const& d)
				{
					return d.name == entry.first;
				});
			}
			if (end == next)
			if (slash or arg.starts_with(dash))
			{
				entry = fmt::to_pair(arg.substr(slash_sz));
				next = std::find_if(begin, end, [&](auto const& d)
				{
					return d.dash == entry.first;
				});
			}

			const bool same = (it == next) or (next == end);

			// Push it either to the command list or as an extra

			if (same) 
			{
				if (0 != argn)
				{
					--argn;
					args.push_back(arg);
				}
				else
				if (not fmt::same(argv[0], arg))
				{
					extra.push_back(arg);
				}
			}
			else // Start parsing the next command if changing
			{
				if (end != it)
				{
					auto const key = make_pair(it->name);
					if (empty(args))
					{
						(void) ptr->put(key, entry.second);
					}
				}

				it = next;
				next = end;
				argn = it->argn;

				if (0 == argn)
				{
					auto const key = make_pair(it->name);
					auto const value = doc::ini::join(args);
					(void) ptr->put(key, value);
				}
			}
		}

		if (end != it)
		{
			auto const key = make_pair(it->name);
			if (empty(args))
			{
				(void) writer->put(key, entry.second);
			}
			else
			{
				auto const value = doc::ini::join(args);
				(void) writer->put(key, value);
			}
		}

		return extra;
	}

	view get(view key)
	{
		span const args = arguments;
		for (auto const arg : args)
		{
			auto const e = fmt::to_pair(arg);
			if (e.first == key)
			{
				return e.second;
			}
		}

		auto value = env::var::get(key);
		if (empty(value))
		{
			auto const entry = make_pair(key);
			value = env::opt::get(entry);
		}
		return value;
	}

	bool got(view key)
	{
		auto const entry = make_pair(key);
		return got(entry);
	}

	bool set(view key, view value)
	{
		auto const entry = make_pair(key);
		return set(entry, value);
	}

	bool put(view key, view value)
	{
		auto const entry = make_pair(key);
		return put(entry, value);
	}

	bool cut(view key)
	{
		auto const entry = make_pair(key);
		return cut(entry);
	}

	view get(pair key)
	{
		auto const ptr = registry().read();
		view::set set;
		while (true)
		{
			view value = ptr->get(key);
			if (value.front() != '@')
			{
				return value;
			}
			if (not set.emplace(value).second)
			{
				sys::warn(here, "Cycle", value);
				break;
			}
			key.second = value.substr(1);
		}
		return fmt::nil;
	}

	bool got(pair key)
	{
		return registry().read()->got(key);
	}

	bool set(pair key, view value)
	{
		return registry().write()->set(key, value);
	}

	bool put(pair key, view value)
	{
		return registry().write()->put(key, value);
	}

	bool cut(pair key)
	{
		return registry().write()->cut(key);
	}

	std::istream & get(std::istream & in)
	{
		auto const unlock = lock.write();
		return in >> registry();
	}

	std::ostream & put(std::ostream & out)
	{
		auto const unlock = lock.read();
		return out << registry();
	}

	bool get(view key, bool value)
	{
		return convert(key, value);
	}

	bool get(pair key, bool value)
	{
		return convert(key, value);
	}

	bool put(pair key, bool value)
	{
		if (value)
		{
			return registry().write()->set(key, "enable");
		}
		else
		{
			return registry().write()->cut(key);
		}
	}

	bool put(view key, bool value)
	{
		auto const entry = make_pair(key);
		return put(entry, value);
	}

	word get(view key, word value, int base)
	{
		return convert(key, value, [base](auto value)
		{
			return fmt::to_llong(value, base);
		});
	}

	word get(pair key, word value, int base)
	{
		return convert(key, value, [base](auto value)
		{
			return fmt::to_llong(value, base);
		});
	}

	bool put(view key, word value, int base)
	{
		return put(key, fmt::to_string(value, base));
	}

	bool put(pair key, word value, int base)
	{
		return put(key, fmt::to_string(value, base));
	}

	quad get(view key, quad value)
	{
		return convert(key, value, [](auto value)
		{
			return fmt::to_quad(value);
		});
	}

	quad get(pair key, quad value)
	{
		return convert(key, value, [](auto value)
		{
			return fmt::to_quad(value);
		});
	}

	bool put(view key, quad value)
	{
		return set(key, fmt::to_string(value));
	}

	bool put(pair key, quad value)
	{
		return set(key, fmt::to_string(value));
	}

	vector get(view key, span value)
	{
		auto const entry = make_pair(key);
		return get(entry, value);
	}

	vector get(pair key, span value)
	{
		view u = get(key);
		if (empty(u))
		{
			return vector(value.begin(), value.end());
		}
		return doc::ini::split(u);
	}

	bool put(view key, span value)
	{
		auto const entry = make_pair(key);
		return put(entry, value);
	}

	bool put(pair key, span value)
	{
		auto const s = doc::ini::join(value);
		return put(key, s);
	}
}
