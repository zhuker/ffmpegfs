/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/**
 * @file
 * @brief Various FFmpegfs utility functions
 *
 * @ingroup ffmpegfs
 *
 * @author Norbert Schlia (nschlia@oblivion-software.de)
 * @copyright Copyright (C) 2017-2019 Norbert Schlia (nschlia@oblivion-software.de)
 */

#ifndef FFMPEG_UTILS_H
#define FFMPEG_UTILS_H

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define __STDC_FORMAT_MACROS                    /**< @brief Force PRId64 defines */

#include "ffmpeg_compat.h"

#include <string>
#include <vector>

#if !defined(USE_LIBSWRESAMPLE) && !defined(USE_LIBAVRESAMPLE)
#error "Must have either libswresample (preferred choice for FFMpeg) or libavresample (with libav)."
#endif

#ifdef USE_LIBSWRESAMPLE
#define LAVR_DEPRECATE                      1   /**< @brief Prefer libswresample */
#else
#define LAVR_DEPRECATE                      0   /**< @brief Prefer libavresample (for Libav libswresample is not available) */
#endif

#ifndef PATH_MAX
#include <linux/limits.h>
#endif

// Disable annoying warnings outside our code
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#ifdef __GNUC__
#  include <features.h>
#  if __GNUC_PREREQ(5,0) || defined(__clang__)
// GCC >= 5.0
#     pragma GCC diagnostic ignored "-Wfloat-conversion"
#  elif __GNUC_PREREQ(4,8)
// GCC >= 4.8
#  else
#     error("GCC < 4.8 not supported");
#  endif
#endif
#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#ifdef __cplusplus
}
#endif
#pragma GCC diagnostic pop

#ifndef LIBAVUTIL_VERSION_MICRO
#error "LIBAVUTIL_VERSION_MICRO not defined. Missing include header?"
#endif

// Libav detection:
// FFmpeg has library micro version >= 100
// Libav  has library micro version < 100
// So if micro < 100, it's Libav, else it's FFmpeg.
#if LIBAVUTIL_VERSION_MICRO < 100
#define USING_LIBAV                         /**< @brief If defined, we are linking to Libav */
#endif

/**
 * Allow use of av_format_inject_global_side_data when available
 */
#define HAVE_AV_FORMAT_INJECT_GLOBAL_SIDE_DATA  (LIBAVFORMAT_VERSION_MICRO >= 100 && LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(57, 64, 101))

/**
 * Add av_get_media_type_string function if missing
 */
#define HAVE_MEDIA_TYPE_STRING              (LIBAVUTIL_VERSION_MICRO >= 100 && LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(55, 34, 101))
#if HAVE_MEDIA_TYPE_STRING
/**
 *  Map to av_get_media_type_string function.
 */
#define get_media_type_string               av_get_media_type_string
#else
/**
 * @brief av_get_media_type_string is missing, so we provide our own.
 * @param[in] media_type - Media type to map.
 * @return Pointer to media type string.
 */
const char *get_media_type_string(enum AVMediaType media_type);
#endif

#ifndef AV_ROUND_PASS_MINMAX
/**
 *  Ignore if this is missing
 */
#define AV_ROUND_PASS_MINMAX            	0
#endif

// These once had a different name
#if !defined(AV_CODEC_CAP_DELAY) && defined(CODEC_CAP_DELAY)
#define AV_CODEC_CAP_DELAY              	CODEC_CAP_DELAY                         /**< @brief AV_CODEC_CAP_DELAY is missing in older FFmpeg versions */
#endif

#if !defined(AV_CODEC_CAP_TRUNCATED) && defined(CODEC_CAP_TRUNCATED)
#define AV_CODEC_CAP_TRUNCATED          	CODEC_CAP_TRUNCATED                     /**< @brief AV_CODEC_CAP_TRUNCATED is missing in older FFmpeg versions */
#endif

#if !defined(AV_CODEC_FLAG_TRUNCATED) && defined(CODEC_FLAG_TRUNCATED)
#define AV_CODEC_FLAG_TRUNCATED         	CODEC_FLAG_TRUNCATED                    /**< @brief AV_CODEC_FLAG_TRUNCATED is missing in older FFmpeg versions */
#endif

#ifndef AV_CODEC_FLAG_GLOBAL_HEADER
#define AV_CODEC_FLAG_GLOBAL_HEADER     	CODEC_FLAG_GLOBAL_HEADER                /**< @brief AV_CODEC_FLAG_GLOBAL_HEADER is missing in older FFmpeg versions */
#endif

#ifndef FF_INPUT_BUFFER_PADDING_SIZE
#define FF_INPUT_BUFFER_PADDING_SIZE    	256                                     /**< @brief FF_INPUT_BUFFER_PADDING_SIZE is missing in newer FFmpeg versions */
#endif

#ifndef AV_CODEC_CAP_VARIABLE_FRAME_SIZE
#define AV_CODEC_CAP_VARIABLE_FRAME_SIZE	CODEC_CAP_VARIABLE_FRAME_SIZE           /**< @brief AV_CODEC_CAP_VARIABLE_FRAME_SIZE is missing in older FFmpeg versions */
#endif

#if !defined(USING_LIBAV) && (LIBAVUTIL_VERSION_MAJOR > 54)
#define BITRATE int64_t                                                             /**< @brief For FFmpeg bit rate is an int64. */
#else // USING_LIBAV
#define BITRATE int                                                                 /**< @brief For FFmpeg bit rate is an int. */
#endif

// Make access possible over codecpar if available
#if LAVF_DEP_AVSTREAM_CODEC || defined(USING_LIBAV)
#define CODECPAR(s)     ((s)->codecpar)                                             /**< @brief Map to codecpar */
#else
#define CODECPAR(s)     ((s)->codec)                                                /**< @brief Map to codec */
#endif

/**
  * File types
  */
typedef enum FILETYPE
{
    FILETYPE_UNKNOWN,
    FILETYPE_MP3,
    FILETYPE_MP4,
    FILETYPE_WAV,
    FILETYPE_OGG,
    FILETYPE_WEBM,
    FILETYPE_MOV,
    FILETYPE_AIFF,
    FILETYPE_OPUS,
    FILETYPE_PRORES,
} FILETYPE;

/**
  * MP4 and MOV prfiles
  */
typedef enum PROFILE
{
    PROFILE_INVALID = -1,       /**< @brief Profile is invalid */

    PROFILE_NONE = 0,           /**< @brief No specific profile/Don't care */

    // MP4
    PROFILE_MP4_FF = 1,         /**< @brief Firefox */
    PROFILE_MP4_EDGE,			/**< @brief MS Edge */
    PROFILE_MP4_IE,				/**< @brief MS Internet Explorer */
    PROFILE_MP4_CHROME,			/**< @brief Google Chrome */
    PROFILE_MP4_SAFARI,			/**< @brief Apple Safari */
    PROFILE_MP4_OPERA,			/**< @brief Opera */
    PROFILE_MP4_MAXTHON,        /**< @brief Maxthon */

    // mov
    PROFILE_MOV_NONE = 0,       /**< @brief Use for all */

    // WEBM

    // prores
    PROFILE_PRORES_NONE = 0,    /**< @brief Use for all */

} PROFILE;

/**
  * Prores levels
  */
typedef enum PRORESLEVEL
{
    PRORESLEVEL_NONE = -1,          /**< @brief No level */
    // Prores profiles
    PRORESLEVEL_PRORES_PROXY = 0,   /**< @brief Prores Level: PROXY */
    PRORESLEVEL_PRORES_LT,          /**< @brief Prores Level: LT */
    PRORESLEVEL_PRORES_STANDARD,    /**< @brief Prores Level: STANDARD */
    PRORESLEVEL_PRORES_HQ,          /**< @brief Prores Level: HQ */
} PRORESLEVEL;

/**
  * Auto copy options
  */
typedef enum AUTOCOPY
{
     AUTOCOPY_OFF = 0,      /**< @brief Never copy streams, transcode always. */
     AUTOCOPY_MATCH,        /**< @brief Copy stream if target supports codec. */
     AUTOCOPY_MATCHLIMIT,   /**< @brief Same as MATCH, only copy if target not larger transcode otherwise. */
     AUTOCOPY_STRICT,       /**< @brief Copy stream if codec matches desired target, transcode otherwise. */
     AUTOCOPY_STRICTLIMIT,  /**< @brief Same as STRICT, only copy if target not larger, transcode otherwise. */
} AUTOCOPY;

/**
 * @brief The #FFmpegfs_Format class
 */
class FFmpegfs_Format
{
public:
    /**
     * @brief Construct FFmpegfs_Format object
     */
    FFmpegfs_Format();
    /**
     * @brief Construct FFmpegfs_Format object
     * @param[in] format_name - Name of this format, e.g. "MP4 file"
     * @param[in] filetype - File type, MP3, MP4, OPUS etc.
     * @param[in] video_codec_id - AVCodec used for video encoding
     * @param[in] audio_codec_id - AVCodec used for audio encoding
     */
    FFmpegfs_Format(const std::string & format_name, FILETYPE filetype, AVCodecID video_codec_id, AVCodecID audio_codec_id);

    /**
     * @brief Get codecs for the selected destination type.
     * @param[in] desttype - Destination type (MP4, WEBM etc.).
     * @return Returns true if format was found; false if not.
     */
    bool                init(const std::string & desttype);
    /**
     * @brief Convert destination type to "real" type, i.e., the file extension to be used.
     * @note Currently "prores" is mapped to "mov".
     * @return Destination type
     */
    const std::string & format_name() const;
    /**
     * @brief Get destination type
     * @return Destination type
     */
    const std::string & desttype() const;
    /**
     * @brief Get selected filetype.
     * @return Returns selected filetype.
     */
    FILETYPE            filetype() const;
    /**
     * @brief Get video codec_id
     * @return Returns video codec_id
     */
    AVCodecID           video_codec_id() const;
    /**
     * @brief Get audio codec_id
     * @return Returns audio codec_id
     */
    AVCodecID           audio_codec_id() const;

protected:
    std::string m_format_name;              /**< @brief Descriptive name of the format, e.g. "Opus Audio". */
    std::string m_desttype;                 /**< @brief Destination type: mp4, mp3 or other */
    FILETYPE    m_filetype;                 /**< @brief File type, MP3, MP4, OPUS etc. */
    AVCodecID   m_video_codec_id;           /**< @brief AVCodec used for video encoding */
    AVCodecID   m_audio_codec_id;           /**< @brief AVCodec used for audio encoding */
};

/**
 * @brief Add / to the path if required
 * @param[in] path - Path to add separator to.
 * @return Returns constant reference to path.
 */
const std::string & append_sep(std::string * path);
/**
 * @brief Add filename to path, including / after the path if required
 * @param[in] path - Path to add filename to.
 * @param[in] filename - File name to add.
 * @return Returns constant reference to path.
 */
const std::string & append_filename(std::string * path, const std::string & filename);
/**
 * @brief Remove / from the path
 * @param[in] path - Path to remove separator from.
 * @return Returns constant reference to path.
 */
const std::string & remove_sep(std::string * path);
/**
 * @brief Remove filename from path. Handy dirname alternative.
 * @param[in] filepath - Path to remove filename from.
 * @return Returns constant reference to path.
 */
const std::string & remove_filename(std::string *filepath);
/**
 * @brief Remove path from filename. Handy basename alternative.
 * @param[in] filepath - Filename to remove path from.
 * @return Returns constant reference to filename.
 */
const std::string & remove_path(std::string *filepath);
/**
 * @brief Remove extension from filename.
 * @param[in] filepath - Filename to remove path from.
 * @return Returns constant reference to filename.
 */
const std::string & remove_ext(std::string *filepath);
/**
 * @brief Find extension in filename, if existing.
 * @param[in] ext - Extension, if found.
 * @param[in] filename - Filename to inspect.
 * @return Returns true if extension was found, false if there was none
 */
bool                find_ext(std::string * ext, const std::string & filename);
/**
 * @brief Replace extension in filename, taking into account that there might not be an extension already.
 * @param[in] filepath - Filename to add extension to.
 * @param[in] ext - Extension to add.
 * @return Returns constant reference to filename.
 */
const std::string & replace_ext(std::string * filepath, const std::string & ext);
/**
 * @brief strdup() variant taking a std::string as input.
 * @param[in] str - String to duplicate.
 * @return Copy of the input string. Remember to delete the allocated memory.
 */
char *              new_strdup(const std::string & str);
/**
 * @brief Get destination filename. Replaces extension and path.
 * @param[in] destfilepath - Destination name and path.
 * @param[in] filepath - Source filename and path.
 * @return Returns constant reference to destfilepath.
 */
const std::string & get_destname(std::string *destfilepath, const std::string & filepath);
/**
 * @brief Get FFmpeg error string for errnum. Internally calls av_strerror().
 * @param[in] errnum - FFmpeg error code.
 * @return Returns std::string with the error defined by errnum.
 */
std::string         ffmpeg_geterror(int errnum);
/**
 * @brief Convert a FFmpeg time from time_base into standard AV_TIME_BASE fractional seconds.
 *
 * Avoids conversion of AV_NOPTS_VALUE.
 *
 * @param[in] ts - Source time.
 * @param[in] time_base - Time base of source.
 * @return Returns converted value, or AV_NOPTS_VALUE if ts is AV_NOPTS_VALUE.
 */
int64_t             ffmpeg_rescale(int64_t ts, const AVRational & time_base);
/**
 * @brief Format numeric value.
 * @param[in] value - Value to format.
 * @return Returns std::string with formatted value; if value == AV_NOPTS_VALUE returns "unset"; "unlimited" if value == 0.
 */
std::string         format_number(int64_t value);
/**
 * @brief Format a bit rate.
 * @param[in] value - Bit rate to format.
 * @return Returns std::string with formatted value in bit/s, kbit/s or Mbit/s. If value == AV_NOPTS_VALUE returns "unset".
 */
std::string         format_bitrate(BITRATE value);
/**
 * @brief Format a samplerate.
 * @param[in] value - Sample rate to format.
 * @return Returns std::string with formatted value in Hz or kHz. If value == AV_NOPTS_VALUE returns "unset".
 */
std::string         format_samplerate(int value);
/**
 * @brief Format a time in format HH:MM:SS.fract
 * @param[in] value - Time value in AV_TIME_BASE factional seconds.
 * @param[in] fracs - Fractional digits.
 * @return Returns std::string with formatted value. If value == AV_NOPTS_VALUE returns "unset".
 */
std::string         format_duration(int64_t value, uint32_t fracs = 3);
/**
 * @brief Format a time in format "w d m s".
 * @param[in] value - Time value in AV_TIME_BASE factional seconds.
 * @return Returns std::string with formatted value. If value == AV_NOPTS_VALUE returns "unset".
 */
std::string         format_time(time_t value);
/**
 * @brief Format size.
 * @param[in] value - Size to format.
 * @return Returns std::string with formatted value in bytes, KB, MB or TB; if value == AV_NOPTS_VALUE returns "unset"; "unlimited" if value == 0.
 */
std::string         format_size(size_t value);
/**
 * @brief Format size.
 * @param[in] value - Size to format.
 * @return Returns std::string with formatted value in bytes plus KB, MB or TB; if value == AV_NOPTS_VALUE returns "unset"; "unlimited" if value == 0.
 */
std::string         format_size_ex(size_t value);
/**
 * @brief Format size of transcoded file including difference between predicted and resulting size.
 * @param[in] size_resulting - Resulting size.
 * @param[in] size_predicted - Predicted size.
 * @return Returns std::string with formatted value in bytes plus difference; if value == AV_NOPTS_VALUE returns "unset"; "unlimited" if value == 0.
 */
std::string         format_result_size(size_t size_resulting, size_t size_predicted);
/**
 * @brief Format size of transcoded file including difference between predicted and resulting size.
 * @param[in] size_resulting - Resulting size.
 * @param[in] size_predicted - Predicted size.
 * @return Returns std::string with formatted value in bytes plus KB, MB or TB and difference; if value == AV_NOPTS_VALUE returns "unset"; "unlimited" if value == 0.
 */
std::string         format_result_size_ex(size_t size_resulting, size_t size_predicted);
/**
 * @brief Path to FFmpegfs binary.
 * @param[in] path - Path to FFmpegfs binary.
 */
void                exepath(std::string *path);
/**
 * @brief trim from start
 * @param[in] s - String to trim.
 * @return Reference to string s.
 */
std::string &       ltrim(std::string &s);
/**
 * @brief trim from end
 * @param[in] s - String to trim.
 * @return Reference to string s.
 */
std::string &       rtrim(std::string &s);
/**
 * @brief trim from both ends
 * @param[in] s - String to trim.
 * @return Reference to string s.
 */
std::string &       trim(std::string &s);
/**
 * @brief Same as std::string replace(), but replaces all occurrences.
 * @param[in] str - Source string.
 * @param[in] from - String to replace.
 * @param[in] to - Replacement string.
 * @return Source string with all occurrences of from replaced with to.
 */
std::string         replace_all(std::string str, const std::string& from, const std::string& to);
/**
 * @brief Format a std::string sprintf-like.
 * @param[in] format - sprintf-like format string.
 * @param[in] args - Arguments.
 * @return Returns the formatted string.
 */
template<typename ... Args> std::string string_format(const std::string& format, Args ... args);
/**
 * @brief strcasecmp() equivalent for std::string.
 * @param[in] s1 - std:string #1
 * @param[in] s2 - std:string #2
 * @return Returns same as strcasecmp() for char *.
 */
int                 strcasecmp(const std::string & s1, const std::string & s2);
/**
 * @brief Get info about the FFmpeg libraries used.
 * @return std::tring with info about the linked FFmpeg libraries.
 */
std::string         ffmpeg_libinfo();
/**
 * @brief Lists all supported codecs and devices.
 * @param[in] device_only - If true lists devices only.
 * @return On success returns 0; on error negative AVERROR.
 */
int                 show_formats_devices(int device_only);
/**
 * @brief Safe way to get the codec name. Function never fails, will return "unknown" on error.
 * @param[in] codec_id - ID of codec
 * @param[in] long_name - If true, gets the long name.
 * @return Returns the codec name or "unknown" on error.
 */
const char *        get_codec_name(AVCodecID codec_id, bool long_name);
/**
 * @brief Check if file type supports album arts.
 * @param[in] filetype - File type: MP3, MP4 etc.
 * @return Returns true if album arts are supported, false if not.
 */
int                 supports_albumart(FILETYPE filetype);
/**
 * @brief Get the FFmpegfs filetype, desttype must be one of FFmpeg's "official" short names for formats.
 * @param[in] desttype - Destination type (MP4, WEBM etc.).
 * @return On success returns FILETYPE enum; on error returns FILETYPE_UNKNOWN.
 */
FILETYPE            get_filetype(const std::string & desttype);
/**
 * @brief Get the FFmpegfs filetype, desttypelist must be a comma separated list of FFmpeg's "official" short names for formats.
 * Will return the first match. Same as get_filetype, but accepts a comma separated list.
 * @param[in] desttypelist - Destination type list (MP4, WEBM etc.) separated by commas.
 * @return On success returns FILETYPE enum; on error returns FILETYPE_UNKNOWN.
 */
FILETYPE            get_filetype_from_list(const std::string & desttypelist);
/**
 * @brief Print info about an AVStream.
 * @param[in] stream - Stream to print.
 * @return On success returns 0; on error negative AVERROR.
 */
int                 print_stream_info(const AVStream* stream);
/**
 * @brief Compare value with pattern.
 * @param[in] value - Value to check.
 * @param[in] pattern - Regexp pattern to match.
 * @return Returns 0 if pattern matches; 1 if not; -1 if pattern is no valid regex
 */
int                 compare(const std::string &value, const std::string &pattern);
/**
 * @brief Expand path, e.g., expand ~/ to home directory.
 * @param[out] tgt - Expanded source path.
 * @param[in] src - Path to expand.
 * @return Omn success, returns expanded source path.
 */
const std::string & expand_path(std::string *tgt, const std::string &src);
/**
 * @brief Check if path is a mount.
 * @param[in] path - Path to check.
 * @return Returns 1 if path is a mount point; 0 if not. On error returns -1. Check errorno for details.
 */
int                 is_mount(const std::string & path);
/**
 * @brief Make directory tree.
 * @param[in] path - Path to create
 * @param[in] mode - Directory mode, see mkdir() function.
 * @return On success, returns 0; on error, returns non-zero errno value.
 */
int                 mktree(const std::string & path, mode_t mode);
/**
 * @brief Get temporary directory.
 * @param[out] path - Path to temporary directory.
 */
void                tempdir(std::string & path);

#ifdef USING_LIBAV
/**
 * @brief Implement missing avformat_alloc_output_context2() for Libav.
 * @param[in] avctx - See FFmpeg avformat_alloc_output_context2() function.
 * @param[in] oformat - See FFmpeg avformat_alloc_output_context2() function.
 * @param[in] format - See FFmpeg avformat_alloc_output_context2() function.
 * @param[in] filename - See FFmpeg avformat_alloc_output_context2() function.
 * @return See FFmpeg avformat_alloc_output_context2() function.
 */
int                 avformat_alloc_output_context2(AVFormatContext **avctx, AVOutputFormat *oformat, const char *format, const char *filename);
/**
 * @brief Implement missing avcodec_get_name for Libav.
 * @param[in] id - See FFmpeg avformat_alloc_output_context2() function.
 * @return See FFmpeg avformat_alloc_output_context2() function.
 */
const char *        avcodec_get_name(AVCodecID id);

/**
 * Create a rational.
 * Useful for compilers that do not support compound literals.
 * @note  The return value is not reduced.
 */
static inline AVRational av_make_q(int num, int den)
{
    AVRational r = { num, den };
    return r;
}
#endif

/**
 * @brief Split string into an array delimited by a regular expression.
 * @param[in] input - Input string.
 * @param[in] regex - Regular expression to match.
 * @return Returns an array with separate elements.
 */
std::vector<std::string> split(const std::string& input, const std::string & regex);

/**
 * Safe countof() implementation: Retuns number of elements in an array.
 */
template <typename T, std::size_t N>
constexpr std::size_t countof(T const (&)[N]) noexcept
{
    return N;
}

/**
 * @brief Sanitise file name. Calls realpath() to remove duplicate // or resolve ../.. etc.
 * @param[in] filepath - File name and path to sanitise.
 * @return Returns sanitised file name and path.
 */
std::string         sanitise_filepath(const std::string & filepath);
/**
 * @brief Sanitise file name. Calls realpath() to remove duplicate // or resolve ../.. etc.
 * Changes the path in place.
 * @param[in] filepath - File name and path to sanitise.
 * @return Returns sanitised file name and path.
 */
std::string         sanitise_filepath(std::string * filepath);

/**
 * @brief Minimal check if codec is an album art.
 * @param[in] codec_id - ID of codec.
 * @return Returns true if codec is for an image; false if not.
 */
bool                is_album_art(AVCodecID codec_id);

/**
 * @brief nocasecompare to make std::string find operations case insensitive
 * @param[in] lhs - left hand string
 * @param[in] rhs - right hand string
 * @return -1 if lhs < rhs; 0 if lhs == rhs and 1 if lhs > rhs
 */
bool                nocasecompare(const std::string & lhs, const std::string &rhs);

/**
 * @brief The comp struct to make std::string find operations case insensitive
 */
struct comp
{
    /**
     * @brief operator () to make std::string find operations case insensitive
     * @param[in] lhs - left hand string
     * @param[in] rhs - right hand string
     * @return -1 if lhs < rhs; 0 if lhs == rhs and 1 if lhs > rhs
     */
    bool operator() (const std::string& lhs, const std::string& rhs) const
    {
        return nocasecompare(lhs, rhs);
    }
};

/**
 * @brief Get free disk space.
 * @param[in] path - Path or file on disk.
 * @return Returns the free disk space.
 */
size_t              get_disk_free(std::string & path);

/**
 * @brief For use with win_smb_fix=1: Check if this an illegal access offset by Windows
 * @param[in] size - sizeof of the file
 * @param[in] offset - offset at which file is accessed
 * @return If request should be ignored, returns true; otherwise false
 */
bool                check_ignore(size_t size, size_t offset);

#endif
