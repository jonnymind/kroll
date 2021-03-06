// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// This file defines utility functions for working with strings.

#ifndef BASE_STRING_UTIL_H_
#define BASE_STRING_UTIL_H_

#include <string>
#include <vector>
#include <stdarg.h>   // va_list

#include "basictypes.h"
#include "string16.h"

// Safe standard library wrappers for all platforms.

namespace base {

// C standard-library functions like "strncasecmp" and "snprintf" that aren't
// cross-platform are provided as "base::strncasecmp", and their prototypes
// are listed below.  These functions are then implemented as inline calls
// to the platform-specific equivalents in the platform-specific headers.

// Compare the two strings s1 and s2 without regard to case using
// the current locale; returns 0 if they are equal, 1 if s1 > s2, and -1 if
// s2 > s1 according to a lexicographic comparison.
int strcasecmp(const char* s1, const char* s2);

// Compare up to count characters of s1 and s2 without regard to case using
// the current locale; returns 0 if they are equal, 1 if s1 > s2, and -1 if
// s2 > s1 according to a lexicographic comparison.
int strncasecmp(const char* s1, const char* s2, size_t count);

// Wrapper for vsnprintf that always null-terminates and always returns the
// number of characters that would be in an untruncated formatted
// string, even when truncation occurs.
int vsnprintf(char* buffer, size_t size, const char* format, va_list arguments);

// vswprintf always null-terminates, but when truncation occurs, it will either
// return -1 or the number of characters that would be in an untruncated
// formatted string.  The actual return value depends on the underlying
// C library's vswprintf implementation.
int vswprintf(wchar_t* buffer, size_t size,
              const wchar_t* format, va_list arguments);

// Some of these implementations need to be inlined.

inline int snprintf(char* buffer, size_t size, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vsnprintf(buffer, size, format, arguments);
  va_end(arguments);
  return result;
}

inline int swprintf(wchar_t* buffer, size_t size, const wchar_t* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vswprintf(buffer, size, format, arguments);
  va_end(arguments);
  return result;
}

// BSD-style safe and consistent string copy functions.
// Copies |src| to |dst|, where |dst_size| is the total allocated size of |dst|.
// Copies at most |dst_size|-1 characters, and always NULL terminates |dst|, as
// long as |dst_size| is not 0.  Returns the length of |src| in characters.
// If the return value is >= dst_size, then the output was truncated.
// NOTE: All sizes are in number of characters, NOT in bytes.
size_t strlcpy(char* dst, const char* src, size_t dst_size);
size_t wcslcpy(wchar_t* dst, const wchar_t* src, size_t dst_size);

// Scan a wprintf format string to determine whether it's portable across a
// variety of systems.  This function only checks that the conversion
// specifiers used by the format string are supported and have the same meaning
// on a variety of systems.  It doesn't check for other errors that might occur
// within a format string.
//
// Nonportable conversion specifiers for wprintf are:
//  - 's' and 'c' without an 'l' length modifier.  %s and %c operate on char
//     data on all systems except Windows, which treat them as wchar_t data.
//     Use %ls and %lc for wchar_t data instead.
//  - 'S' and 'C', which operate on wchar_t data on all systems except Windows,
//     which treat them as char data.  Use %ls and %lc for wchar_t data
//     instead.
//  - 'F', which is not identified by Windows wprintf documentation.
//  - 'D', 'O', and 'U', which are deprecated and not available on all systems.
//     Use %ld, %lo, and %lu instead.
//
// Note that there is no portable conversion specifier for char data when
// working with wprintf.
//
// This function is intended to be called from base::vswprintf.
bool IsWprintfFormatPortable(const wchar_t* format);

}  // namespace base

#if defined(OS_WIN32)
#include "string_util_win.h"
#elif defined(OS_POSIX)
#include "base/string_util_posix.h"
#else
#error Define string operations appropriately for your platform
#endif

// Returns a reference to a globally unique empty string that functions can
// return.  Use this to avoid static construction of strings, not to replace
// any and all uses of "std::string()" as nicer-looking sugar.
// These functions are threadsafe.
const std::string& EmptyString();
const std::wstring& EmptyWString();

extern const wchar_t kWhitespaceWide[];
extern const char kWhitespaceASCII[];

// Names of codepages (charsets) understood by icu.
extern const char* const kCodepageUTF8;

// Removes characters in trim_chars from the beginning and end of input.
// NOTE: Safe to use the same variable for both input and output.
bool TrimString(const std::wstring& input,
                const wchar_t trim_chars[],
                std::wstring* output);
bool TrimString(const std::string& input,
                const char trim_chars[],
                std::string* output);

// Trims any whitespace from either end of the input string.  Returns where
// whitespace was found.  The non-wide version of this function only looks for
// ASCII whitespace; UTF-8 code-points are not searched for (use the wide
// version instead).
// NOTE: Safe to use the same variable for both input and output.
enum TrimPositions {
  TRIM_NONE     = 0,
  TRIM_LEADING  = 1 << 0,
  TRIM_TRAILING = 1 << 1,
  TRIM_ALL      = TRIM_LEADING | TRIM_TRAILING,
};
TrimPositions TrimWhitespace(const std::wstring& input,
                             TrimPositions positions,
                             std::wstring* output);
TrimPositions TrimWhitespace(const std::string& input,
                             TrimPositions positions,
                             std::string* output);

// Searches  for CR or LF characters.  Removes all contiguous whitespace
// strings that contain them.  This is useful when trying to deal with text
// copied from terminals.
// Returns |text, with the following three transformations:
// (1) Leading and trailing whitespace is trimmed.
// (2) If |trim_sequences_with_line_breaks| is true, any other whitespace
//     sequences containing a CR or LF are trimmed.
// (3) All other whitespace sequences are converted to single spaces.
std::wstring CollapseWhitespace(const std::wstring& text,
                                          bool trim_sequences_with_line_breaks);

// These convert between ASCII (7-bit) and UTF16 strings.
std::string KROLL_API WideToASCII(const std::wstring& wide);
std::wstring KROLL_API ASCIIToWide(const std::string& ascii);

// These convert between UTF-8, -16, and -32 strings. They are potentially slow,
// so avoid unnecessary conversions. The low-level versions return a boolean
// indicating whether the conversion was 100% valid. In this case, it will still
// do the best it can and put the result in the output buffer. The versions that
// return strings ignore this error and just return the best conversion
// possible.
KROLL_API bool WideToUTF8(const wchar_t* src, size_t src_len, std::string* output);
KROLL_API std::string WideToUTF8(const std::wstring& wide);
KROLL_API bool UTF8ToWide(const char* src, size_t src_len, std::wstring* output);
KROLL_API std::wstring UTF8ToWide(const std::string& utf8);

bool KROLL_API WideToUTF16(const wchar_t* src, size_t src_len, string16* output);
string16 KROLL_API WideToUTF16(const std::wstring& wide);
bool KROLL_API UTF16ToWide(const char16* src, size_t src_len, std::wstring* output);
std::wstring KROLL_API UTF16ToWide(const string16& utf8);

bool KROLL_API UTF8ToUTF16(const char* src, size_t src_len, string16* output);
string16 KROLL_API UTF8ToUTF16(const std::string& utf8);
bool KROLL_API UTF16ToUTF8(const char16* src, size_t src_len, std::string* output);
std::string KROLL_API UTF16ToUTF8(const string16& utf16);

// Defines the error handling modes of WideToCodepage and CodepageToWide.
class OnStringUtilConversionError {
 public:
  enum Type {
    // The function will return failure. The output buffer will be empty.
    FAIL,

    // The offending characters are skipped and the conversion will proceed as
    // if they did not exist.
    SKIP,
  };

 private:
  OnStringUtilConversionError();
};

// Converts between wide strings and the encoding specified.  If the
// encoding doesn't exist or the encoding fails (when on_error is FAIL),
// returns false.
bool WideToCodepage(const std::wstring& wide,
                              const char* codepage_name,
                              OnStringUtilConversionError::Type on_error,
                              std::string* encoded);
bool CodepageToWide(const std::string& encoded,
                              const char* codepage_name,
                              OnStringUtilConversionError::Type on_error,
                              std::wstring* wide);

// Converts the given wide string to the corresponding Latin1. This will fail
// (return false) if any characters are more than 255.
bool WideToLatin1(const std::wstring& wide, std::string* latin1);

// Returns true if the specified string matches the criteria. How can a wide
// string be 8-bit or UTF8? It contains only characters that are < 256 (in the
// first case) or characters that use only 8-bits and whose 8-bit
// representation looks like a UTF-8 string (the second case).
bool IsString8Bit(const std::wstring& str);
bool IsStringUTF8(const std::string& str);
bool IsStringWideUTF8(const std::wstring& str);
bool IsStringASCII(const std::wstring& str);
bool IsStringASCII(const std::string& str);

// ASCII-specific tolower.  The standard library's tolower is locale sensitive,
// so we don't want to use it here.
template <class Char> inline Char ToLowerASCII(Char c) {
  return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

// Converts the elements of the given string.  This version uses a pointer to
// clearly differentiate it from the non-pointer variant.
template <class str> inline void StringToLowerASCII(str* s) {
  for (typename str::iterator i = s->begin(); i != s->end(); ++i)
    *i = ToLowerASCII(*i);
}

template <class str> inline str StringToLowerASCII(const str& s) {
  // for std::string and std::wstring
  str output(s);
  StringToLowerASCII(&output);
  return output;
}

// Compare the lower-case form of the given string against the given ASCII
// string.  This is useful for doing checking if an input string matches some
// token, and it is optimized to avoid intermediate string copies.  This API is
// borrowed from the equivalent APIs in Mozilla.
bool LowerCaseEqualsASCII(const std::string& a, const char* b);
bool LowerCaseEqualsASCII(const std::wstring& a, const char* b);

// Same thing, but with string iterators instead.
bool LowerCaseEqualsASCII(std::string::const_iterator a_begin,
                          std::string::const_iterator a_end,
                          const char* b);
bool LowerCaseEqualsASCII(std::wstring::const_iterator a_begin,
                          std::wstring::const_iterator a_end,
                          const char* b);
bool LowerCaseEqualsASCII(const char* a_begin,
                          const char* a_end,
                          const char* b);
bool LowerCaseEqualsASCII(const wchar_t* a_begin,
                          const wchar_t* a_end,
                          const char* b);

// Returns true if str starts with search, or false otherwise.
bool StartsWithASCII(const std::string& str,
                     const std::string& search,
                     bool case_sensitive);
bool StartsWith(const std::wstring& str,
                const std::wstring& search,
                bool case_sensitive);

// Determines the type of ASCII character, independent of locale (the C
// library versions will change based on locale).
template <typename Char>
inline bool IsAsciiWhitespace(Char c) {
  return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}
template <typename Char>
inline bool IsAsciiAlpha(Char c) {
  return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'));
}
template <typename Char>
inline bool IsAsciiDigit(Char c) {
  return c >= '0' && c <= '9';
}

// Returns true if it's a whitespace character.
inline bool IsWhitespace(wchar_t c) {
  return wcschr(kWhitespaceWide, c) != NULL;
}

// TODO(mpcomplete): Decide if we should change these names to KIBI, etc,
// or if we should actually use metric units, or leave as is.
enum DataUnits {
  DATA_UNITS_BYTE = 0,
  DATA_UNITS_KILOBYTE,
  DATA_UNITS_MEGABYTE,
  DATA_UNITS_GIGABYTE,
};

// Return the unit type that is appropriate for displaying the amount of bytes
// passed in.
DataUnits GetByteDisplayUnits(int64 bytes);

// Return a byte string in human-readable format, displayed in units appropriate
// specified by 'units', with an optional unit suffix.
// Ex: FormatBytes(512, DATA_UNITS_KILOBYTE, true) => "0.5 KB"
// Ex: FormatBytes(10*1024, DATA_UNITS_MEGABYTE, false) => "0.1"
std::wstring FormatBytes(int64 bytes, DataUnits units, bool show_units);

// As above, but with "/s" units.
// Ex: FormatSpeed(512, DATA_UNITS_KILOBYTE, true) => "0.5 KB/s"
// Ex: FormatSpeed(10*1024, DATA_UNITS_MEGABYTE, false) => "0.1"
std::wstring FormatSpeed(int64 bytes, DataUnits units, bool show_units);

// Return a number formated with separators in the user's locale way.
// Ex: FormatNumber(1234567) => 1,234,567
std::wstring FormatNumber(int64 number);

// Starting at |start_offset| (usually 0), look through |str| and replace all
// instances of |find_this| with |replace_with|.
//
// This does entire substrings; use std::replace in <algorithm> for single
// characters, for example:
//   std::replace(str.begin(), str.end(), 'a', 'b');
void ReplaceSubstringsAfterOffset(std::wstring* str,
                                            std::wstring::size_type start_offset,
                                            const std::wstring& find_this,
                                            const std::wstring& replace_with);
void ReplaceSubstringsAfterOffset(std::string* str,
                                  std::string::size_type start_offset,
                                  const std::string& find_this,
                                  const std::string& replace_with);

// Specialized string-conversion functions.
std::string IntToString(int value);
std::wstring IntToWString(int value);
std::string UintToString(unsigned int value);
std::wstring UintToWString(unsigned int value);
std::string Int64ToString(int64 value);
std::wstring Int64ToWString(int64 value);
std::string Uint64ToString(uint64 value);
std::wstring Uint64ToWString(uint64 value);
// The DoubleToString methods convert the double to a string format that
// ignores the locale.  If you want to use locale specific formatting, use ICU.
std::string DoubleToString(double value);
std::wstring DoubleToWString(double value);

// Perform a best-effort conversion of the input string to a numeric type,
// setting |*output| to the result of the conversion.  Returns true for
// "perfect" conversions; returns false in the following cases:
//  - Overflow/underflow.  |*output| will be set to the maximum value supported
//    by the data type.
//  - Trailing characters in the string after parsing the number.  |*output|
//    will be set to the value of the number that was parsed.
//  - No characters parseable as a number at the beginning of the string.
//    |*output| will be set to 0.
//  - Empty string.  |*output| will be set to 0.
bool StringToInt(const std::string& input, int* output);
bool StringToInt(const std::wstring& input, int* output);
bool StringToInt64(const std::string& input, int64* output);
bool StringToInt64(const std::wstring& input, int64* output);
bool HexStringToInt(const std::string& input, int* output);
bool HexStringToInt(const std::wstring& input, int* output);

// For floating-point conversions, only conversions of input strings in decimal
// form are defined to work.  Behavior with strings representing floating-point
// numbers in hexadecimal, and strings representing non-fininte values (such as
// NaN and inf) is undefined.  Otherwise, these behave the same as the integral
// variants.  This expects the input string to NOT be specific to the locale.
// If your input is locale specific, use ICU to read the number.
bool StringToDouble(const std::string& input, double* output);
bool StringToDouble(const std::wstring& input, double* output);

// Convenience forms of the above, when the caller is uninterested in the
// boolean return value.  These return only the |*output| value from the
// above conversions: a best-effort conversion when possible, otherwise, 0.
int StringToInt(const std::string& value);
int StringToInt(const std::wstring& value);
int64 StringToInt64(const std::string& value);
int64 StringToInt64(const std::wstring& value);
int HexStringToInt(const std::string& value);
int HexStringToInt(const std::wstring& value);
double StringToDouble(const std::string& value);
double StringToDouble(const std::wstring& value);

// Return a C++ string given printf-like input.
std::string StringPrintf(const char* format, ...);
std::wstring StringPrintf(const wchar_t* format, ...);

// Store result into a supplied string and return it
const std::string& SStringPrintf(std::string* dst, const char* format, ...);
const std::wstring& SStringPrintf(std::wstring* dst,
                                  const wchar_t* format, ...);

// Append result to a supplied string
void StringAppendF(std::string* dst, const char* format, ...);
void StringAppendF(std::wstring* dst, const wchar_t* format, ...);

// Lower-level routine that takes a va_list and appends to a specified
// string.  All other routines are just convenience wrappers around it.
void StringAppendV(std::string* dst, const char* format, va_list ap);
void StringAppendV(std::wstring* dst, const wchar_t* format, va_list ap);

// This is mpcomplete's pattern for saving a string copy when dealing with
// a function that writes results into a wchar_t[] and wanting the result to
// end up in a std::wstring.  It ensures that the std::wstring's internal
// buffer has enough room to store the characters to be written into it, and
// sets its .length() attribute to the right value.
//
// The reserve() call allocates the memory required to hold the string
// plus a terminating null.  This is done because resize() isn't
// guaranteed to reserve space for the null.  The resize() call is
// simply the only way to change the string's 'length' member.
//
// XXX-performance: the call to wide.resize() takes linear time, since it fills
// the string's buffer with nulls.  I call it to change the length of the
// string (needed because writing directly to the buffer doesn't do this).
// Perhaps there's a constant-time way to change the string's length.
template <class char_type>
inline char_type* WriteInto(
    std::basic_string<char_type, std::char_traits<char_type>,
                      std::allocator<char_type> >* str,
    size_t length_including_null) {
  str->reserve(length_including_null);
  str->resize(length_including_null - 1);
  return &((*str)[0]);
}

inline char16* WriteInto(string16* str, size_t length_including_null) {
  str->reserve(length_including_null);
  str->resize(length_including_null - 1);
  return &((*str)[0]);
}

//-----------------------------------------------------------------------------

// Function objects to aid in comparing/searching strings.

template<typename Char> struct CaseInsensitiveCompare {
 public:
  bool operator()(Char x, Char y) const {
    return tolower(x) == tolower(y);
  }
};

template<typename Char> struct CaseInsensitiveCompareASCII {
 public:
  bool operator()(Char x, Char y) const {
    return ToLowerASCII(x) == ToLowerASCII(y);
  }
};

//-----------------------------------------------------------------------------

// Splits |str| into a vector of strings delimited by |s|. Append the results
// into |r| as they appear. If several instances of |s| are contiguous, or if
// |str| begins with or ends with |s|, then an empty string is inserted.
//
// Every substring is trimmed of any leading or trailing white space.
void SplitString(const std::wstring& str,
                 wchar_t s,
                 std::vector<std::wstring>* r);
void SplitString(const std::string& str,
                 char s,
                 std::vector<std::string>* r);

// The same as SplitString, but don't trim white space.
void SplitStringDontTrim(const std::wstring& str,
                         wchar_t s,
                         std::vector<std::wstring>* r);
void SplitStringDontTrim(const std::string& str,
                         char s,
                         std::vector<std::string>* r);

// WARNING: this uses whitespace as defined by the HTML5 spec. If you need
// a function similar to this but want to trim all types of whitespace, then
// factor this out into a function that takes a string containing the characters
// that are treated as whitespace.
//
// Splits the string along whitespace (where whitespace is the five space
// characters defined by HTML 5). Each contiguous block of non-whitespace
// characters is added to result.
void SplitStringAlongWhitespace(const std::wstring& str,
                                std::vector<std::wstring>* result);

// Replace $1-$2-$3 in the format string with |a| and |b| respectively.
// Additionally, $$ is replaced by $. The offset/offsets parameter here can be
// NULL.
std::wstring ReplaceStringPlaceholders(const std::wstring& format_string,
                                       const std::wstring& a,
                                       size_t* offset);

std::wstring ReplaceStringPlaceholders(const std::wstring& format_string,
                                       const std::wstring& a,
                                       const std::wstring& b,
                                       std::vector<size_t>* offsets);

std::wstring ReplaceStringPlaceholders(const std::wstring& format_string,
                                       const std::wstring& a,
                                       const std::wstring& b,
                                       const std::wstring& c,
                                       std::vector<size_t>* offsets);

std::wstring ReplaceStringPlaceholders(const std::wstring& format_string,
                                       const std::wstring& a,
                                       const std::wstring& b,
                                       const std::wstring& c,
                                       const std::wstring& d,
                                       std::vector<size_t>* offsets);

// If the size of |input| is more than |max_len|, this function returns true and
// |input| is shortened into |output| by removing chars in the middle (they are
// replaced with up to 3 dots, as size permits).
// Ex: ElideString(L"Hello", 10, &str) puts Hello in str and returns false.
// ElideString(L"Hello my name is Tom", 10, &str) puts "Hell...Tom" in str and
// returns true.
bool ElideString(const std::wstring& input, int max_len, std::wstring* output);

// Returns true if the string passed in matches the pattern. The pattern
// string can contain wildcards like * and ?
// TODO(iyengar) This function may not work correctly for CJK strings as
// it does individual character matches.
// The backslash character (\) is an escape character for * and ?
bool MatchPattern(const std::wstring& string, const std::wstring& pattern);
bool MatchPattern(const std::string& string, const std::string& pattern);

#endif  // BASE_STRING_UTIL_H_

