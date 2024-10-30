#pragma once
#include "Common/Containers/String.h"
#include "Common/Containers/StringView.h"
#include "Common/Containers/Vector.h"
#include "Common/Containers/DateTime.h"
#include <cstdint>

namespace StringUtils
{

enum Compare {
    CaseSensitive=0,
    CaseInsensitive
};
[[nodiscard]] inline String fmt(const char *fmt,...) _FORMAT_ATTRIBUTE(1,2);

[[nodiscard]] inline String fmt(const char *fmt,...)
{
    va_list ap;
    va_start(ap, fmt);
    String ret;
    ret.append_sprintf_va_list(fmt, ap);
    va_end(ap);
    return ret;
}
[[nodiscard]] double to_double(StringView p_str, bool* ok = nullptr);
[[nodiscard]] double to_double(const char *p_str, char ** r_end);
[[nodiscard]] inline float to_float(StringView str,bool *ok=nullptr) { return float(to_double(str, ok)); }
//[[nodiscard]] int hex_to_int(StringView s,bool p_with_prefix=true);
//[[nodiscard]] int to_int(const char *p_str, int p_len = -1);
[[nodiscard]] int to_int(StringView p_str,bool *ok=nullptr);
//[[nodiscard]] int64_t to_int64(StringView str);

//[[nodiscard]] String format(StringView str,const Variant &values);

template<typename T>
[[nodiscard]] bool contains(const Vector<T> &container,StringView needle,Compare case_sensitive=CaseSensitive) {
    for(const T& entry : container) {
        if(0==compare(entry,needle,case_sensitive))
            return true;
    }
    return false;
}

[[nodiscard]] String num(double p_num, int p_decimals = -1, bool trailing_zeroes=true);
[[nodiscard]] String num_scientific(double p_num, int p_decimals = -1);
[[nodiscard]] String num_real(double p_num);
[[nodiscard]] String num_int64(int64_t p_num, int base = 10, bool capitalize_hex = false);
[[nodiscard]] String num_uint64(uint64_t p_num, int base = 10, bool capitalize_hex = false);
[[nodiscard]] String md5(const uint8_t *p_md5);
[[nodiscard]] String hex_encode_buffer(const uint8_t *p_buffer, int p_len);

[[nodiscard]] float similarity(StringView lhs,StringView p_string);
[[nodiscard]] Vector<StringView> bigrams(StringView str);
[[nodiscard]] int compare(StringView lhs,StringView rhs,Compare case_sensitive=CaseSensitive);
[[nodiscard]] String percent_encode(StringView src);
[[nodiscard]] String percent_decode(StringView str);

[[nodiscard]] String md5_text(StringView str);
[[nodiscard]] String sha1_text(StringView str);
[[nodiscard]] String sha256_text(StringView str);
//[[nodiscard]] String sha256_text(const String &str);
[[nodiscard]] FixedVector<uint8_t, 16, false> md5_buffer(StringView str);

[[nodiscard]] Vector<uint8_t> sha1_buffer(StringView str);
[[nodiscard]] Vector<uint8_t> sha256_buffer(StringView str);

[[nodiscard]] char char_lowercase(char p_char);
[[nodiscard]] char char_uppercase(char p_char);

[[nodiscard]] String to_lower(StringView str);
[[nodiscard]] String to_upper(StringView str);

[[nodiscard]] String xml_escape(StringView arg, bool p_escape_quotes=false);
[[nodiscard]] String xml_unescape(StringView src);

[[nodiscard]] String xml_unescape(StringView src);
[[nodiscard]] String http_escape(StringView src);
//[[nodiscard]] String http_unescape(StringView src);
[[nodiscard]] String c_escape(StringView src);
[[nodiscard]] String c_escape_multiline(StringView src);
[[nodiscard]] String c_unescape(StringView src);
[[nodiscard]] String json_escape(StringView src);
[[nodiscard]] String property_name_encode(StringView str);
[[nodiscard]] DateTime parseDate(const char *data, StringView fmt);

[[nodiscard]] bool is_subsequence_of(StringView str,StringView p_string, Compare mode=CaseSensitive);

//[[nodiscard]] UIString repeat(const UIString &str,int p_count);
[[nodiscard]] Vector<StringView> rsplit(StringView str,StringView p_splitter, bool p_allow_empty = true, int p_maxsplit = 0);
[[nodiscard]] Vector<StringView> split(StringView str,StringView p_splitter, bool p_allow_empty = true, int p_maxsplit = 0);
[[nodiscard]] Vector<StringView> split(StringView str,char p_splitter, bool p_allow_empty = true);
[[nodiscard]] Vector<StringView> split_any(StringView str,StringView split_chars,bool p_allow_empty=true);
//[[nodiscard]] Vector<float> split_floats(const String &str,const String &p_splitter, bool p_allow_empty = true);
[[nodiscard]] Vector<float> split_floats(StringView str,StringView p_splitter, bool p_allow_empty = true);
[[nodiscard]] Vector<float> split_floats_mk(StringView str,StringView split_chars, bool p_allow_empty = true);
[[nodiscard]] Vector<StringView> split_spaces(StringView str);
[[nodiscard]] bool contains(StringView heystack, char c);
[[nodiscard]] bool contains(StringView heystack, StringView c,Compare mode=CaseSensitive);
[[nodiscard]] bool is_numeric(StringView str);
[[nodiscard]] bool is_quoted(StringView str);
[[nodiscard]] bool is_valid_filename(StringView src);
//[[nodiscard]] bool is_valid_float(const String &src);
[[nodiscard]] bool is_valid_float(StringView src);
[[nodiscard]] bool is_valid_int(StringView src);
[[nodiscard]] bool is_valid_hex_number(StringView src,bool p_with_prefix);
[[nodiscard]] bool is_valid_integer(StringView str);
//[[nodiscard]] bool is_valid_ip_address(StringView src);
[[nodiscard]] String repeat(StringView str,int p_count);

[[nodiscard]] String quote(StringView str,char character = '\"');
[[nodiscard]] StringView unquote(StringView str);

inline size_t hash(const char *p_cstr, int p_len)
{
    return eastl::hash<StringView>()(StringView(p_cstr,p_len));
}
inline size_t hash(const char *p_cstr)
{
    return eastl::hash<StringView>()(StringView(p_cstr));
}

[[nodiscard]] bool begins_with(StringView s,StringView p_string);
[[nodiscard]] bool begins_with(StringView s,StringView p_string,Compare mode);
[[nodiscard]] bool ends_with(StringView s,StringView p_string);
[[nodiscard]] bool ends_with(StringView s,StringView p_string,Compare mode);
[[nodiscard]] bool ends_with(StringView s,char c);

//[[nodiscard]] int64_t hex_to_int64(StringView s,bool p_with_prefix = true);
//[[nodiscard]] int64_t bin_to_int64(StringView str,bool p_with_prefix = true);

[[nodiscard]] String capitalize(StringView str);
[[nodiscard]] String camelcase_to_underscore(StringView str,bool lowercase = true);
[[nodiscard]] String to_snake_case(StringView str);
[[nodiscard]] String validate_identifier(StringView str);
[[nodiscard]] String to_pascal_case(const String &self);
[[nodiscard]] String to_camel_case(const String &self);
[[nodiscard]] String to_snake_case(const String &self);

[[nodiscard]] int count(StringView heystack,StringView p_string, int p_from = 0, int p_to = 0);
[[nodiscard]] int countn(StringView heystack,StringView p_string, int p_from = 0, int p_to = 0);

[[nodiscard]] int get_slice_count(StringView str,char p_splitter);
[[nodiscard]] int get_slice_count(StringView str,StringView p_splitter);

[[nodiscard]] StringView get_slice(StringView str,char p_splitter, int p_slice);
[[nodiscard]] StringView get_slice(StringView str,StringView p_splitter, int p_slice);

//[[nodiscard]] String dedent(const String &str);
[[nodiscard]] bool match(StringView str,StringView p_wildcard,Compare sensitivity=CaseSensitive);
[[nodiscard]] bool matchn(StringView str,StringView p_wildcard);
[[nodiscard]] size_t find(StringView str,StringView p_str, size_t p_from = 0); ///< return String::npos if failed
[[nodiscard]] size_t find(StringView str,char p_str, size_t p_from = 0); ///< return String::npos if failed
[[nodiscard]] size_t findn(StringView str,StringView p_str, int p_from = 0); ///< return <0 if failed, case insensitive
[[nodiscard]] size_t rfind(StringView str,StringView p_str, int p_from = -1); ///< return <0 if failed
[[nodiscard]] size_t rfind(StringView str,char p_str, int p_from = -1); ///< return <0 if failed
[[nodiscard]] StringView left(StringView s,int p_pos);
[[nodiscard]] StringView right(StringView str,int p_pos);
[[nodiscard]] String strip_escapes(StringView tr);
[[nodiscard]] StringView lstrip(StringView str,StringView p_chars);
[[nodiscard]] StringView rstrip(StringView str,StringView p_chars);
[[nodiscard]] StringView strip_edges(StringView str, bool left = true, bool right = true);
[[nodiscard]] StringView substr(StringView str,int p_from, size_t p_chars = size_t(-1));
[[nodiscard]] String dedent(StringView str);

// Grapheme and locale specific helpers
[[nodiscard]] int grapheme_count(StringView str);
[[nodiscard]] int bytes_in_next_grapheme(StringView str,int offset=0);


/* complex helpers */
void erase(String &str,int p_pos, int p_chars);

[[nodiscard]] String replace(StringView str,StringView p_key, StringView p_with);
[[nodiscard]] String replace(StringView str,char p_key, char p_with);

[[nodiscard]] String insert(StringView str,int p_at_pos, StringView p_string);
[[nodiscard]] String lpad(const String &src,int min_length, char character = ' ');
[[nodiscard]] String pad_decimals(StringView str,int p_digits);
[[nodiscard]] String pad_zeros(StringView str,int p_digits);
[[nodiscard]] String replace_first(StringView str, StringView p_key, StringView p_with);
[[nodiscard]] String replacen(StringView str,StringView p_key, StringView p_with);
[[nodiscard]] String rpad(const String &src,int min_length, char character = ' ');
[[nodiscard]] StringView trim_prefix(StringView str,StringView p_prefix);
[[nodiscard]] StringView trim_suffix(StringView str,StringView p_suffix);
[[nodiscard]] String simplified(StringView);

} // end of StringUtils namespace

namespace PathUtils
{
    [[nodiscard]] StringView get_extension(StringView p);
    [[nodiscard]] StringView get_basename(StringView p);
    //! Returns a path to give file: /a/path/to/file -> /a/path/to  file -> .
    [[nodiscard]] StringView path(StringView p);
    [[nodiscard]] String plus_file(StringView bp,StringView p_file);
    [[nodiscard]] String join_path(Span<StringView> parts);
    [[nodiscard]] String join_path(std::initializer_list<StringView> parts);
    [[nodiscard]] String join_path(StringView p1,StringView p2);
    //! @note for now it just replaces \\ with /
    [[nodiscard]] String from_native_path(StringView p);
    [[nodiscard]] String to_win_path(StringView v);
    // path functions
    [[nodiscard]] bool is_internal_path(StringView path);
    [[nodiscard]] bool is_abs_path(StringView p);
    [[nodiscard]] bool is_rel_path(StringView p);
    [[nodiscard]] String path_to(StringView p, StringView  p_path);
    [[nodiscard]] String path_to_file(StringView p, StringView  p_path);
    [[nodiscard]] String get_base_dir(StringView path);
    [[nodiscard]] StringView get_file(StringView path);
    [[nodiscard]] StringView trim_trailing_slash(StringView path);
    [[nodiscard]] String simplify_path(StringView str);
    //[[nodiscard]] String trim_trailing_slash(const String &path);
    [[nodiscard]] bool is_network_share_path(StringView path);

} // end o PathUtils namespace

namespace CharUtils {
inline bool is_digit(char c) {
    return c >= '0' && c <= '9';
}
inline bool is_hex_digit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}
inline bool is_ascii_char(char c) {
    return (c>='a' && c<='z') || (c>='A' && c<='Z');
}
inline bool is_underscore(char c) {
    return c == '_';
}
inline bool is_ascii_upper_case(char c) {
    return eastl::CharToUpper(c) == c;
}
inline bool is_ascii_lower_case(char c) {
    return eastl::CharToLower(c) == c;
}
inline bool is_ascii_identifier_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}
inline bool is_symbol(char c) {
    return c != '_' && ((c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '`') || (c >= '{' && c <= '~') || c == '\t' || c == ' ');
}
inline bool is_whitespace(char p_char) {
    return p_char == ' ' || p_char == '\t' || (p_char >= 0x0009 && p_char <= 0x000d);
}
inline bool is_linebreak(char32_t p_char) {
    return (p_char >= 0x000a && p_char <= 0x000d) || (p_char == 0x0085) || (p_char == 0x2028) || (p_char == 0x2029);
}

inline bool is_punct(char32_t p_char) {
    return (p_char >= ' ' && p_char <= '/') || (p_char >= ':' && p_char <= '@') || (p_char >= '[' && p_char <= '^') || (p_char == '`') || (p_char >= '{' && p_char <= '~') || (p_char >= 0x2000 && p_char <= 0x206f) || (p_char >= 0x3000 && p_char <= 0x303f);
}
inline bool is_control(char32_t p_char) {
    return (p_char <= 0x001f) || (p_char >= 0x007f && p_char <= 0x009f);
}
} // namespace CharUtils

String itos(int64_t p_val);
String rtos(double p_val);
String rtoss(double p_val); // scientific version

/* end of namespace */

