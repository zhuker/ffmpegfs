/*
 * Logging class header for ffmpegfs
 *
 * Copyright (C) 2017-2018 K. Henriksson
 *
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

#ifndef LOGGING_H
#define LOGGING_H

#pragma once

#include <map>
#include <fstream>
#include <sstream>
#include <regex>

#include <iostream>

class Logging
{
public:
    enum class level
    {
        ERROR = 1,
        WARNING = 2,
        INFO = 3,
        DEBUG = 4,
        TRACE = 5
    };

    /*
     * Arguments:
     *   logfile: The name of a file to write logging output to. If empty, no
     *     output will be written.
     *   max_level: The maximum level of log output to write.
     *   to_stderr: Whether to write log output to stderr.
     *   to_syslog: Whether to write log output to syslog.
     */
    explicit Logging(const std::string & logfile, level max_level, bool to_stderr, bool to_syslog);

    bool GetFail() const
    {
        return m_logfile.fail();
    }

    //private:
    class Logger : public std::ostringstream
    {
    public:
        Logger(level loglevel, const std::string & filename, Logging* logging) :
            m_loglevel(loglevel),
            m_filename(filename),
            m_logging(logging) {}
        Logger() :
            m_loglevel(level::DEBUG) {}
        virtual ~Logger();

    private:
        const level m_loglevel;

        const std::string   m_filename;
        Logging*            m_logging;

        static const std::map<level, int>           m_syslog_level_map;
        static const std::map<level, std::string>   m_level_name_map;
        static const std::map<level, std::string>   m_level_colour_map;
    };
public:
    static bool init_logging(const std::string & logfile, Logging::level max_level, bool to_stderr, bool to_syslog);

    template <typename... Args>
    static void trace(const char *filename, const std::string &format_string, Args &&...args)
    {
        log_with_level(Logging::level::TRACE, filename != nullptr ? filename : "", format_helper(format_string, 1, std::forward<Args>(args)...));
    }
    template <typename... Args>
    static void trace(const std::string &filename, const std::string &format_string, Args &&...args)
    {
        log_with_level(Logging::level::TRACE, filename, format_helper(format_string, 1, std::forward<Args>(args)...));
    }

    template <typename... Args>
    static void debug(const char * filename, const std::string &format_string, Args &&...args)
    {
        log_with_level(Logging::level::DEBUG, filename != nullptr ? filename : "", format_helper(format_string, 1, std::forward<Args>(args)...));
    }
    template <typename... Args>
    static void debug(const std::string & filename, const std::string &format_string, Args &&...args)
    {
        log_with_level(Logging::level::DEBUG, filename, format_helper(format_string, 1, std::forward<Args>(args)...));
    }

    template <typename... Args>
    static void info(const char *filename, const std::string &format_string, Args &&...args)
    {
        log_with_level(Logging::level::INFO, filename != nullptr ? filename : "", format_helper(format_string, 1, std::forward<Args>(args)...));
    }
    template <typename... Args>
    static void info(const std::string &filename, const std::string &format_string, Args &&...args)
    {
        log_with_level(Logging::level::INFO, filename, format_helper(format_string, 1, std::forward<Args>(args)...));
    }

    template <typename... Args>
    static void warning(const char *filename, const std::string &format_string, Args &&...args)
    {
        log_with_level(Logging::level::WARNING, filename != nullptr ? filename : "", format_helper(format_string, 1, std::forward<Args>(args)...));
    }
    template <typename... Args>
    static void warning(const std::string &filename, const std::string &format_string, Args &&...args)
    {
        log_with_level(Logging::level::WARNING, filename, format_helper(format_string, 1, std::forward<Args>(args)...));
    }

    template <typename... Args>
    static void error(const char *filename, const std::string &format_string, Args &&...args)
    {
        log_with_level(Logging::level::ERROR, filename != nullptr ? filename : "", format_helper(format_string, 1, std::forward<Args>(args)...));
    }
    template <typename... Args>
    static void error(const std::string &filename, const std::string &format_string, Args &&...args)
    {
        log_with_level(Logging::level::ERROR, filename, format_helper(format_string, 1, std::forward<Args>(args)...));
    }

    static void log_with_level(Logging::level level, const std::string & filename, const std::string & message);

protected:
    static std::string format_helper(
            const std::string &string_to_update,
            const size_t);

    template <typename T, typename... Args>
    static std::string format_helper(
            const std::string &string_to_update,
            const size_t index_to_replace,
            T &&val,
            Args &&...args) {
        std::regex pattern("%" + std::to_string(index_to_replace) + "(?=[^0-9])");
        std::ostringstream ostr;
        ostr << val;
        std::string replacement_string(ostr.str());
        return format_helper(
                    std::regex_replace(string_to_update, pattern, replacement_string),
                    index_to_replace + 1,
                    std::forward<Args>(args)...);
    }

    template <typename... Args>
    static std::string format(const std::string &format_string, Args &&...args)
    {
        return format_helper(format_string, 1, std::forward<Args>(args)...);
    }

protected:
    friend Logger Log(level lev, const std::string & filename);
    friend Logger;

    std::ofstream   m_logfile;
    const level     m_max_level;
    const bool      m_to_stderr;
    const bool      m_to_syslog;
};

constexpr auto ERROR    = Logging::level::ERROR;
constexpr auto WARNING  = Logging::level::WARNING;
constexpr auto INFO     = Logging::level::INFO;
constexpr auto DEBUG    = Logging::level::DEBUG;
constexpr auto TRACE    = Logging::level::TRACE;

#endif
