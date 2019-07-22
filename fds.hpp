#ifndef fds_hpp
#define fds_hpp

#include <iostream>
#include "file.hpp"
#include "buf.hpp"
#include "fmt.hpp"

namespace sys::io
{
	//
	// Abstract buffer class
	//

	template
	<
	 class Char,
	 template <class> class Traits = std::char_traits
	 template <class> class Alloc = std::allocator
	>
	class basic_fdbuf : public basic_membuf<Char, Traits, Alloc>
	{
		using base = basic_membuf<Char, Traits, Alloc>;

	public:

		using size_type = typename base::size_type;
		using char_type = typename base::char_type;

		basic_fdbuf(int fd = -1)
		{
			set(fd);
		}

		int set(int fd = -1)
		{
			return file.set(fd);
		}

		bool close()
		{
			return file.close();
		}

		bool is_open() const
		{
			return !!file;
		}

	protected:

		sys::file::descriptor file;

		size_type xsputn(char_type const *s, size_type n) override
		{
			return file.write(s, sizeof (char_type) * n);
		}

		size_type xsgetn(char_type *s, size_type n) override
		{
			return file.read(s, sizeof (char_type) * n);
		}
	};

	// Common alias types

	using fdbuf = basic_fdbuf<char>;
	using wfdbuf = basic_fdbuf<wchar_t>;

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
		 template <class, class> basic_stream,
		 sys::file::openmode default_mode
		>
		class basic_fdstream
		: public basic_fdbuf<Char, Traits, Alloc>
		, public basic_stream<Char, Traits<Char>>
		{
			using base = basic_stream<Char, Traits<Char>>;
			using fdbuf = basic_fdbuf<Char, Traits, Alloc>;
			using string = std::basic_string<Char, Traits<Char>, Alloc<Char>>;
			using string_view = fmt::basic_string_view<Char, Traits<Char>>;
			using openmode = sys::file::openmode;

		public:

			basic_fdstream(std::size_t sz = sys::file::bufsiz, int fd = -1)
			: size(sz)
			, fdbuf(fd)
			, base(this)
			{ }

			basic_fdstream(string_view path, openmode mode = default_mode)
			: basic_fdstream()
			{
				open(path, mode);
			}

			void open(string_view path, openmode mode = default_mode)
			{
				if (mode & sys::file::out and mode & sys::file::in)
				{
					fdbuf::setbufsiz(size, size);
				}
				else if (mode & sys::file::out)
				{
					fdbuf::setbufsiz(0, size);
				}
				else if (mode & sys::file::in)
				{
					fdbuf::setbufsiz(size, 0);
				}

				fmt::string const s = fmt::to_string(path);
				this->file.open(s, mode | default_mode);
			}

		private:

			std::size_t const size;
		};
	}

	// Common alias types

	template
	<
	 class Char,
	 template <class> class Traits = std::char_traits,
	 template <class> class Alloc = std::allocator
	>
	using basic_fdstream = impl::basic_fdstream
	<
	 Char, Traits, Alloc,
	 std::basic_iostream,
	 sys::file::in|sys::file::out
	>;

	using fdstream = basic_fdstream<char>;
	using wfdstream = basic_fdstream<wchar_t>;

	template
	<
	 class Char,
	 template <class> class Traits = std::char_traits,
	 template <class> class Alloc = std::allocator
	>
	using basic_ifdstream = impl::basic_fdstream
	<
	 Char, Traits, Alloc,
	 std::basic_istream,
	 sys::file::in
	>;

	using ifdstream = basic_ifdstream<char>;
	using wifdstream = basic_ifdstream<wchar_t>;

	template
	<
	 class Char,
	 template <class> class Traits = std::char_traits,
	 template <class> class Alloc = std::allocator
	>
	using basic_ofdstream = impl::basic_fdstream
	<
	 Char, Traits, Alloc,
	 std::basic_ostream,
	 sys::file::out
	>;

	using ofdstream = basic_ofdstream<char>;
	using wofdstream = basic_ofdstream<wchar_t>;
}

#endif // file