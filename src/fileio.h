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
 * @brief FileIO class
 *
 * This class allows transparent access to files from DVD, Bluray, Video CD or
 * to regular disk files.
 *
 * @ingroup ffmpegfs
 *
 * @author Norbert Schlia (nschlia@oblivion-software.de)
 * @copyright Copyright (C) 2017-2019 Norbert Schlia (nschlia@oblivion-software.de)
 */

#ifndef FILEIO_H
#define FILEIO_H

#pragma once

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/stat.h>
#include <string>

/** @brief Virtual file types enum
 */
typedef enum VIRTUALTYPE
{
    VIRTUALTYPE_PASSTHROUGH,                                        /**< @brief passthrough file, not used */
    VIRTUALTYPE_REGULAR,                                            /**< @brief Regular file to transcode */
    VIRTUALTYPE_SCRIPT,                                             /**< @brief Virtual script */
    VIRTUALTYPE_BUFFER,                                             /**< @brief Buffer file */
#ifdef USE_LIBVCD
    VIRTUALTYPE_VCD,                                                /**< @brief Video CD file */
#endif // USE_LIBVCD
#ifdef USE_LIBDVD
    VIRTUALTYPE_DVD,                                                /**< @brief DVD file */
#endif // USE_LIBDVD
#ifdef USE_LIBBLURAY
    VIRTUALTYPE_BLURAY,                                             /**< @brief Bluray disk file */
#endif // USE_LIBBLURAY
} VIRTUALTYPE;
typedef VIRTUALTYPE const *LPCVIRTUALTYPE;                          /**< @brief Pointer version of VIRTUALTYPE */
typedef VIRTUALTYPE LPVIRTUALTYPE;                                  /**< @brief Pointer to const version of VIRTUALTYPE */

/** @brief Virtual file definition
 */
typedef struct VIRTUALFILE
{
    VIRTUALFILE()
        : m_type(VIRTUALTYPE_REGULAR)
        , m_format_idx(0)
        , m_full_title(false)
        , m_duration(0)
    {

    }

    VIRTUALTYPE     m_type;                                         /**< @brief Type of this virtual file */

    int             m_format_idx;                                   /**< @brief Index into params.format[] array */
    std::string     m_origfile;                                     /**< @brief Sanitised original file name */
    struct stat     m_st;                                           /**< @brief stat structure with size etc. */

    bool            m_full_title;                                   /**< @brief If true, ignore m_chapter_no and provide full track */
    int64_t         m_duration;                                     /**< @brief Track/chapter duration, in AV_TIME_BASE fractional seconds. */

#ifdef USE_LIBVCD
    /** @brief Extra value structure for Video CDs
     *  @note Only available if compiled with -DUSE_LIBVCD
     */
    struct VCD_CHAPTER
    {
        VCD_CHAPTER()
            : m_track_no(0)
            , m_chapter_no(0)
            , m_start_pos(0)
            , m_end_pos(0)
        {}
        int         m_track_no;                                     /**< @brief Track number (1..) */
        int         m_chapter_no;                                   /**< @brief Chapter number (1..) */
        uint64_t    m_start_pos;                                    /**< @brief Start offset in bytes */
        uint64_t    m_end_pos;                                      /**< @brief End offset in bytes (not including this byte) */
    }               m_vcd;                                          /**< @brief S/VCD track/chapter info */
#endif //USE_LIBVCD
#ifdef USE_LIBDVD
    /** @brief Extra value structure for DVDs
     *  @note Only available if compiled with -DUSE_LIBDVD
     */
    struct DVD_CHAPTER
    {
        DVD_CHAPTER()
            : m_title_no(0)
            , m_chapter_no(0)
            , m_angle_no(0)
        {}
        int         m_title_no;                                     /**< @brief Track number (1...n) */
        int         m_chapter_no;                                   /**< @brief Chapter number (1...n) */
        int         m_angle_no;                                     /**< @brief Selected angle number (1...n) */
    }               m_dvd;                                          /**< @brief DVD title/chapter info */
#endif // USE_LIBDVD
#ifdef USE_LIBBLURAY
    /** @brief Extra value structure for Bluray Disks
     *  @note Only available if compiled with -DUSE_LIBBLURAY
     */
    struct BLURAY_CHAPTER
    {
        BLURAY_CHAPTER()
            : m_title_no(0)
            , m_playlist_no(0)
            , m_chapter_no(0)
            , m_angle_no(0)
        {}
        uint32_t    m_title_no;                                     /**< @brief Track number (1...n) */
        uint32_t    m_playlist_no;                                  /**< @brief Playlist number (1...n) */
        unsigned    m_chapter_no;                                   /**< @brief Chapter number (1...n) */
        unsigned    m_angle_no;                                     /**< @brief Selected angle number (1...n) */
    }               m_bluray;                                       /**< @brief Bluray title/chapter info */
#endif // USE_LIBBLURAY

} VIRTUALFILE;
typedef VIRTUALFILE const *LPCVIRTUALFILE;                          /**< @brief Pointer to const version of VIRTUALFILE */
typedef VIRTUALFILE *LPVIRTUALFILE;                                 /**< @brief Pointer version of VIRTUALFILE */

/** @brief Base class for I/O
 */
class FileIO
{
public:
    /**
     * @brief Create #FileIO object
     */
    explicit FileIO();
    /**
     * @brief Free #FileIO object
     */
    virtual ~FileIO();

    /** @brief Allocate the correct object for type().
     *
     * Free with delete if no longer required.
     *
     * @param[in] type - VIRTUALTYPE of new object
     * @return Upon successful completion, #FileIO of requested type. @n
     * On error (out of memory), returns a nullptr.
     */
    static FileIO * alloc(VIRTUALTYPE type);

    /**
     * @brief Get type of the virtual file
     * @return Returns the type of the virtual file.
     */
    virtual VIRTUALTYPE type() const = 0;
    /**
     * @brief Get the ideal buffer size.
     * @return Return the ideal buffer size.
     */
    virtual size_t  	bufsize() const = 0;
    /** @brief Open a virtual file
     * @param[in] virtualfile - LPCVIRTUALFILE of file to open
     * @return Upon successful completion, #open() returns 0. @n
     * On error, an nonzero value is returned and errno is set to indicate the error.
     */
    virtual int         open(LPCVIRTUALFILE virtualfile);
    /** @brief Read data from file
     * @param[out] data - buffer to store read bytes in. Must be large enough to hold up to size bytes.
     * @param[in] size - number of bytes to read
     * @return Upon successful completion, #read() returns the number of bytes read. @n
     * This may be less than size. @n
     * On error, the value 0 is returned and errno is set to indicate the error. @n
     * If at end of file, 0 may be returned by errno not set. error() will return 0 if at EOF.
     */
    virtual size_t      read(void *data, size_t size) = 0;
    /**
     * @brief Get last error.
     * @return errno value of last error.
     */
    virtual int         error() const = 0;
    /** @brief Get the duration of the file, in AV_TIME_BASE fractional seconds.
     *
     * This is only possible for file formats that are aware of the play time.
     * May be AV_NOPTS_VALUE if the time is not known.
     */
    virtual int64_t     duration() const = 0;
    /**
     * @brief Get the file size.
     * @return Returns the file size.
     */
    virtual size_t      size() const = 0;
    /**
     * @brief Get current read position.
     * @return Gets the current read position.
     */
    virtual size_t      tell() const = 0;
    /** @brief Seek to position in file
     *
     * Repositions the offset of the open file to the argument offset according to the directive whence.
     *
     * @param[in] offset - offset in bytes
     * @param[in] whence - how to seek: @n
     * SEEK_SET: The offset is set to offset bytes. @n
     * SEEK_CUR: The offset is set to its current location plus offset bytes. @n
     * SEEK_END: The offset is set to the size of the file plus offset bytes.
     * @return Upon successful completion, #seek() returns the resulting offset location as measured in bytes
     * from the beginning of the file.  @n
     * On error, the value -1 is returned and errno is set to indicate the error.
     */
    virtual int         seek(long offset, int whence) = 0;
    /**
     * @brief Check if at end of file.
     * @return Returns true if at end of file.
     */
    virtual bool        eof() const = 0;
    /**
     * @brief Close virtual file.
     */
    virtual void        close() = 0;
    /**
     * @brief Get virtual file object
     * @return Current virtual file object or nullptr if unset.
     */
    LPCVIRTUALFILE      virtualfile() const;

protected:
    /** @brief Open a virtual file
     * @param[in] filename - Name of file to open. Only required for disk files.
     * @return Upon successful completion, #openX() returns 0. @n
     * On error, an nonzero value is returned and errno is set to indicate the error.
     */
    virtual int         openX(const std::string & filename) = 0;
    /**
     * @brief Set the internal source path
     * @param[in] path - Source path, with or without file name
     * @return Source path without file name
     */
    const std::string & set_path(const std::string & path);

protected:
    std::string         m_path;                                     /**< @brief Source path (directory without file name) */

private:
    LPCVIRTUALFILE      m_virtualfile;                              /**< @brief Virtual file object of current file */
};

#endif // FILEIO_H
