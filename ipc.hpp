#ifndef ipc_hpp
#define ipc_hpp

#include <vector>
#include <initializer_list>
#include <iostream>
#include "file.hpp"
#include "buf.hpp"

namespace sys::io
{
	//
	// Abstract buffer class
	//

	template
	<
	 class Char,
	 template <class> class Traits = std::char_traits,
	 template <class> class Alloc = std::allocator
	>
	class basic_procbuf : public basic_membuf<Char, Traits, Alloc>
	{
		using base = basic_membuf<Char, Traits, Alloc>;

	public:

		using char_type = typename base::char_type;
		using size_type = typename base::size_type;
		using arguments = std::initializer_list<char const*>;

		void set(int fd[3] = nullptr)
		{
			file.set(fd);
		}

		bool execute(arguments args)
		{
			std::vector<char const*> argv(args);
			argv.push_back(nullptr); // terminator
			return file.execute(argv.data());
		}

		void terminate()
		{
			file.terminate();
		}

		void close(int n)
		{
			file.close(n);
		}

	protected:

		sys::file::process file;

		size_type xsputn(char_type const *s, size_type n) override
		{
			auto const sz = n * sizeof (char_type);
			return file[0].write(s, to_size(sz));
		}

		size_type xsgetn(char_type *s, size_type n) override
		{
			auto const sz = n * sizeof (char_type);
			return file[1].read(s, to_size(sz));
		}
	};

	// Common alias types

	using procbuf = basic_procbuf<char>;
	using wprocbuf = basic_procbuf<wchar_t>;

	//
	// Abstract stream class
	//

	namespace impl
	{
		template
		<
		 class Char,
		 template <class> class Traits,
		 template <class> class Alloc,
		 template <class, class> class basic_stream
		>
		class basic_pstream
		: public basic_procbuf<Char, Traits, Alloc>
		, public basic_stream<Char, Traits<Char>>
		{
			using base = basic_stream<Char, Traits<Char>>;
			using procbuf = basic_procbuf<Char, Traits, Alloc>;
			using arguments = typename procbuf::arguments;

		public:

			basic_pstream(std::size_t sz = sys::file::bufsiz)
			: procbuf()
			, base(this)
			{
				this->setbufsiz(sz);
			}

			basic_pstream(arguments args)
			: basic_pstream()
			{
				this->execute(args);
			}
		};
	}

	// Common alias types

	template
	<
	 class Char,
	 template <class> class Traits = std::char_traits,
	 template <class> class Alloc = std::allocator
	>
	using basic_pstream = impl::basic_pstream
	<
	 Char, Traits, Alloc,
	 std::basic_iostream
	>;
	
	using pstream = basic_pstream<char>;
	using wpstream = basic_pstream<wchar_t>;

	template
	<
	 class Char,
	 template <class> class Traits = std::char_traits,
	 template <class> class Alloc = std::allocator
	>
	using basic_ipstream = impl::basic_pstream
	<
	 Char, Traits, Alloc,
	 std::basic_istream
	>;

	using ipstream = basic_ipstream<char>;
	using wipstream = basic_ipstream<wchar_t>;

	template 
	<
	 class Char,
	 template <class> class Traits = std::char_traits,
	 template <class> class Alloc = std::allocator
	>
	using basic_opstream = impl::basic_pstream
	<
	 Char, Traits, Alloc,
	 std::basic_ostream
	>;

	using opstream = basic_opstream<char>;
	using wopstream = basic_opstream<wchar_t>;
}

#endif // file
