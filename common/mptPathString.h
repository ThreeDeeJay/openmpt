/*
 * mptPathString.h
 * ---------------
 * Purpose: Wrapper class around the platform-native representation of path names. Should be the only type that is used to store path names.
 * Notes  : Currently none.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/namespace.hpp"
#include "mpt/path/os_path.hpp"
#include "mpt/string/types.hpp"
#include "mpt/string_transcode/transcode.hpp"

#include "mptString.h"



OPENMPT_NAMESPACE_BEGIN



namespace mpt
{



#if defined(MPT_ENABLE_CHARSET_LOCALE)
using RawPathString = mpt::os_path;
#define MPT_PATHSTRING_LITERAL(x) MPT_OSPATH_LITERAL( x )
#define MPT_PATHSTRING(x) mpt::PathString::FromNative(MPT_OSPATH_LITERAL( x ))
#else // !MPT_ENABLE_CHARSET_LOCALE
using RawPathString = mpt::utf8string;
#define MPT_PATHSTRING_LITERAL(x) ( x )
#define MPT_PATHSTRING(x) mpt::PathString::FromNative( x )
#endif // MPT_ENABLE_CHARSET_LOCALE

#define PC_(x) MPT_PATHSTRING_LITERAL(x)
#define PL_(x) MPT_PATHSTRING_LITERAL(x)
#define P_(x) MPT_PATHSTRING(x)



struct NativePathTraits
{

	static bool IsPathSeparator(RawPathString::value_type c);

	static RawPathString::value_type GetDefaultPathSeparator();

	static void SplitPath(RawPathString path, RawPathString *prefix, RawPathString *drive, RawPathString *dir, RawPathString *fbase, RawPathString *fext);

	static RawPathString Simplify(const RawPathString &path);

};



class PathString
{

private:

	using path_traits = NativePathTraits;

	RawPathString path;

private:

	explicit PathString(const RawPathString & path_)
		: path(path_)
	{
		return;
	}

public:

	PathString() = default;
	PathString(const PathString &) = default;
	PathString(PathString &&) noexcept = default;

	PathString & assign(const PathString & other)
	{
		path = other.path;
		return *this;
	}

	PathString & assign(PathString && other) noexcept
	{
		path = std::move(other.path);
		return *this;
	}
	
	PathString & operator = (const PathString & other)
	{
		return assign(other);
	}
	
	PathString &operator = (PathString && other) noexcept
	{
		return assign(std::move(other));
	}
	
	PathString & append(const PathString & other)
	{
		path.append(other.path);
		return *this;
	}
	
	PathString & operator += (const PathString & other)
	{
		return append(other);
	}

	friend PathString operator + (const PathString & a, const PathString & b)
	{
		return PathString(a).append(b);
	}

	friend bool operator < (const PathString & a, const PathString & b)
	{
		return a.AsNative() < b.AsNative();
	}

	friend bool operator == (const PathString & a, const PathString & b)
	{
		return a.AsNative() == b.AsNative();
	}
	
	friend bool operator != (const PathString & a, const PathString & b)
	{
		return a.AsNative() != b.AsNative();
	}

	bool empty() const
	{
		return path.empty();
	}

	std::size_t Length() const
	{
		return path.size();
	}

public:

	RawPathString AsNative() const
	{
		return path;
	}
	
	static PathString FromNative(const RawPathString &path)
	{
		return PathString(path);
	}

	mpt::ustring ToUnicode() const
	{
		return mpt::transcode<mpt::ustring>(path);
	}

	static PathString FromUnicode(const mpt::ustring &path)
	{
		return PathString(mpt::transcode<mpt::RawPathString>(path));
	}

	std::string ToUTF8() const
	{
		return mpt::transcode<std::string>(mpt::common_encoding::utf8, path);
	}

	static PathString FromUTF8(const std::string &path)
	{
		return PathString(mpt::transcode<mpt::RawPathString>(mpt::common_encoding::utf8, path));
	}

#if MPT_WSTRING_CONVERT

	std::wstring ToWide() const
	{
		return mpt::transcode<std::wstring>(path);
	}

	static PathString FromWide(const std::wstring &path)
	{
		return PathString(mpt::transcode<mpt::RawPathString>(path));
	}

#endif // MPT_WSTRING_CONVERT

#if defined(MPT_ENABLE_CHARSET_LOCALE)

	std::string ToLocale() const
	{
		return mpt::transcode<std::string>(mpt::logical_encoding::locale, path);
	}

	static PathString FromLocale(const std::string &path)
	{
		return PathString(mpt::transcode<mpt::RawPathString>(mpt::logical_encoding::locale, path));
	}

#endif // MPT_ENABLE_CHARSET_LOCALE

#if defined(MPT_WITH_MFC)

	CString ToCString() const
	{
		return mpt::transcode<CString>(path);
	}

	static PathString FromCString(const CString &path)
	{
		return PathString(mpt::transcode<mpt::RawPathString>(path));
	}

#endif // MPT_WITH_MFC

public:

	static bool IsPathSeparator(RawPathString::value_type c)
	{
		return path_traits::IsPathSeparator(c);
	}

	static RawPathString::value_type GetDefaultPathSeparator()
	{
		return path_traits::GetDefaultPathSeparator();
	}

	bool HasTrailingSlash() const
	{
		if(path.empty())
		{
			return false;
		}
		RawPathString::value_type c = path[path.length() - 1];
		return IsPathSeparator(c);
	}

	PathString WithoutTrailingSlash() const
	{
		PathString result = *this;
		while(result.HasTrailingSlash())
		{
			if(result.Length() == 1)
			{
				return result;
			}
			result = PathString(result.AsNative().substr(0, result.AsNative().length() - 1));
		}
		return result;
	}

	PathString WithTrailingSlash() const
	{
		PathString result = *this;
		if(!result.empty() && !result.HasTrailingSlash())
		{
			result.path += GetDefaultPathSeparator();
		}
		return result;
	}

	void SplitPath(PathString *prefix, PathString *drive, PathString *dir, PathString *fbase, PathString *fext) const
	{
		path_traits::SplitPath(path, prefix ? &prefix->path : nullptr, drive ? &drive->path : nullptr, dir ? &dir->path : nullptr, fbase ? &fbase->path : nullptr, fext ? &fext->path : nullptr);
	}

	// \\?\ or \\?\\UNC or empty
	PathString GetPrefix() const
	{
		PathString prefix;
		SplitPath(&prefix, nullptr, nullptr, nullptr, nullptr);
		return prefix;
	}
	
	// Drive letter + colon, e.g. "C:" or \\server\share
	PathString GetDrive() const
	{
		PathString drive;
		SplitPath(nullptr, &drive, nullptr, nullptr, nullptr);
		return drive;
	}
	
	// Directory, e.g. "\OpenMPT\"
	PathString GetDirectory() const
	{
		PathString dir;
		SplitPath(nullptr, nullptr, &dir, nullptr, nullptr);
		return dir;
	}
	
	// Drive + Dir, e.g. "C:\OpenMPT\"
	PathString GetDirectoryWithDrive() const
	{
		PathString drive, dir;
		SplitPath(nullptr, &drive, &dir, nullptr, nullptr);
		return drive + dir;
	}
	
	// File name without extension, e.g. "OpenMPT"
	PathString GetFilenameBase() const
	{
		PathString fname;
		SplitPath(nullptr, nullptr, nullptr, &fname, nullptr);
		return fname;
	}
	
	// Extension including dot, e.g. ".exe"
	PathString GetFilenameExtension() const
	{
		PathString ext;
		SplitPath(nullptr, nullptr, nullptr, nullptr, &ext);
		return ext;
	}
	
	// File name + extension, e.g. "OpenMPT.exe"
	PathString GetFilename() const
	{
		PathString name, ext;
		SplitPath(nullptr, nullptr, nullptr, &name, &ext);
		return name + ext;
	}

	// Return the same path string with a different (or appended) extension (including "."), e.g. "foo.bar",".txt" -> "foo.txt" or "C:\OpenMPT\foo",".txt" -> "C:\OpenMPT\foo.txt"
	PathString ReplaceExtension(const PathString &newExt) const
	{
		return GetDirectoryWithDrive() + GetFilenameBase() + newExt;
	}

	// Convert a path to its simplified form, i.e. remove ".\" and "..\" entries, similar to std::fs::path::lexically_normal
	PathString Simplify() const
	{
		return PathString::FromNative(path_traits::Simplify(path));
	}

	bool IsAbsolute() const;

#if defined(MODPLUG_TRACKER) && MPT_OS_WINDOWS

	// Relative / absolute paths conversion

	PathString AbsolutePathToRelative(const PathString &relativeTo) const; // similar to std::fs::path::lexically_approximate
	
	PathString RelativePathToAbsolute(const PathString &relativeTo) const;

#endif // MODPLUG_TRACKER && MPT_OS_WINDOWS

};



#if defined(MPT_ENABLE_CHARSET_LOCALE)
inline std::string ToAString(const mpt::PathString &x)
{
	return x.ToLocale();
}
#else
inline std::string ToAString(const mpt::PathString &x)
{
	return x.ToUTF8();
}
#endif
inline mpt::ustring ToUString(const mpt::PathString &x)
{
	return x.ToUnicode();
}
#if MPT_WSTRING_FORMAT
inline std::wstring ToWString(const mpt::PathString &x)
{
	return x.ToWide();
}
#endif



// Return native string, with possible \\?\ prefix if it exceeds MAX_PATH characters.
mpt::RawPathString SupportLongPath(const mpt::RawPathString &path);



#if MPT_OS_WINDOWS
#if !MPT_OS_WINDOWS_WINRT
int PathCompareNoCase(const PathString &a, const PathString &b);
#endif // !MPT_OS_WINDOWS_WINRT
#endif



#if MPT_OS_WINDOWS

#if !(MPT_OS_WINDOWS_WINRT && (_WIN32_WINNT < 0x0a00))

// Returns the absolute path for a potentially relative path and removes ".." or "." components. (same as GetFullPathNameW)
mpt::PathString GetAbsolutePath(const mpt::PathString &path);

#endif

#endif // MPT_OS_WINDOWS



} // namespace mpt



#if defined(MODPLUG_TRACKER)

// Sanitize a filename (remove special chars)

mpt::PathString SanitizePathComponent(const mpt::PathString &filename);

std::string SanitizePathComponent(std::string str);
std::wstring SanitizePathComponent(std::wstring str);
#if MPT_USTRING_MODE_UTF8
mpt::u8string SanitizePathComponent(mpt::u8string str);
#endif // MPT_USTRING_MODE_UTF8
#if defined(MPT_WITH_MFC)
CString SanitizePathComponent(CString str);
#endif // MPT_WITH_MFC

#endif // MODPLUG_TRACKER



OPENMPT_NAMESPACE_END
