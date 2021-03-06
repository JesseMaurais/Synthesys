#ifndef fs_hpp
#define fs_hpp "File Descriptor Stream"

#include "pipe.hpp"
#include "io.hpp"
#include "type.hpp"

namespace fmt
{
	namespace impl
	{
		template
		<
			class Char,
			template <class> class Traits,
			template <class> class Alloc,
			template 
			<
				class,
				template <class> class,
				template <class> class
			> class Stream,
			auto Default
		>
		class basic_fdstream : public Stream<Char, Traits, Alloc>
		{
			using base = Stream<Char, Traits, Alloc>;
			using string = fmt::basic_string<Char, Traits, Alloc>;
			using view = fmt::basic_string_view<Char, Traits>;
			using size_t = env::file::size_t;
			using mode = env::file::mode;
			inline auto width = env::file::width;

			env::file::descriptor f;

		public:

			basic_fdstream(mode mask = Default, size_t size = width())
			: base(f)
			{
				if (mask & env::file::rw)
				{
					base::setbufsiz(size, size);
				}
				else 
				if (mask & env::file::wr)
				{
					base::setbufsiz(0, size);
				}
				else 
				if (mask & env::file::rd)
				{
					base::setbufsiz(size, 0);
				}
			}

			basic_fdstream(view path, mode mask = Default, size_t size = width())
			: base(f), f(path, mode(mask | Default))
			{
				if (mask & env::file::rw)
				{
					base::setbufsiz(size, size);
				}
				else 
				if (mask & env::file::wr)
				{
					base::setbufsiz(0, size);
				}
				else 
				if (mask & env::file::rd)
				{
					base::setbufsiz(size, 0);
				}
			}

			bool open(view path, mode mask = Default)
			{
				return f.open(path, mode(mask | Default));
			}

			void close()
			{
				f.close();
			}

			int set(int fd)
			{
				return f.set(fd);
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
	using basic_fdstream = impl::basic_fdstream
	<
		Char, Traits, Alloc, basic_iostream, env::file::rw
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
		Char, Traits, Alloc, basic_istream, env::file::rd
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
		Char, Traits, Alloc, basic_ostream, env::file::wr
	>;

	using ofdstream = basic_ofdstream<char>;
	using wofdstream = basic_ofdstream<wchar_t>;
}

#endif // file
