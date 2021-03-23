/*
 * (c) 2021, Petr Tesarik <ptesarik@suse.de>, SUSE Linux Software Solutions GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef CPIO_H
#define CPIO_H

#include <ostream>
#include <string>
#include <map>
#include <memory>

//{{{ CPIOMember ---------------------------------------------------------------

class CPIOMember {

    protected:
        static const unsigned long MODE_MASK = 07777UL;

        unsigned long m_ino;
        unsigned long m_mode;
        unsigned long m_uid;
        unsigned long m_gid;
        unsigned long m_nlink;
        unsigned long m_mtime;
        unsigned long m_filesize;
        unsigned long m_devmajor;
        unsigned long m_devminor;
        unsigned long m_rdevmajor;
        unsigned long m_rdevminor;
        std::string m_name;

    public:
        CPIOMember(std::string const &name, unsigned long mode);

        unsigned long ino() const
        { return m_ino; }

        void mode(unsigned long val)
        { m_mode = (val & MODE_MASK) | (m_mode & ~MODE_MASK); }
        unsigned long mode() const
        { return m_mode; }

        void uid(unsigned long val)
        { m_uid = val; }
        unsigned long uid() const
        { return m_uid; }

        void gid(unsigned long val)
        { m_gid = val; }
        unsigned long gid() const
        { return m_gid; }

        unsigned long nLink() const
        { return m_nlink; }

        void mtime(unsigned long val)
        { m_mtime = val; }
        unsigned long mtime() const
        { return m_mtime; }

        unsigned long fileSize() const
        { return m_filesize; }

        unsigned long devMajor() const
        { return m_devmajor; }

        unsigned long devMinor() const
        { return m_devminor; }

        unsigned long rDevMajor() const
        { return m_rdevmajor; }

        unsigned long rDevMinor() const
        { return m_rdevminor; }

        std::string const& name() const
        { return m_name; }

        virtual void writeData(std::ostream &os) const = 0;
};

//}}}
//{{{ CPIOTrailer --------------------------------------------------------------

class CPIOTrailer : public CPIOMember {
    public:
        CPIOTrailer();

        virtual void writeData(std::ostream &os) const;
};

//}}}
//{{{ CPIOSynth ----------------------------------------------------------------

/**
 * Base class for synthesized CPIO members.
 */
class CPIOSynth : public CPIOMember {
    private:
        static int m_lastino;

    public:
        CPIOSynth(std::string const &name, unsigned long mode)
            : CPIOMember(name, mode)
        { m_ino = ++m_lastino; }
};

//}}}
//{{{ CPIODirectory ------------------------------------------------------------

class CPIODirectory : public CPIOSynth {

    public:
        CPIODirectory(std::string const &name, unsigned long mode = 0755);

        virtual void writeData(std::ostream &os) const;
};

//}}}
//{{{ CPIOMemory ---------------------------------------------------------------

class CPIOMemory : public CPIOSynth {

    protected:
        const char *m_buf;

    public:
        CPIOMemory(std::string const &name, const char *buf, size_t len,
                   unsigned long mode = 0644);

        virtual void writeData(std::ostream &os) const;
};

//}}}
//{{{ CPIOFile -----------------------------------------------------------------

class CPIOFile : public CPIOMember {

    protected:
        std::string m_srcpath;

    public:
        CPIOFile(std::string const &name, std::string const &srcpath);
        CPIOFile(std::string const &path)
            : CPIOFile(path, path)
        { }

        virtual void writeData(std::ostream &os) const;
};

//}}}
//{{{ CPIO_newc ----------------------------------------------------------------

/**
 * Producer of the new (SVR4) portable format, understood by the Linux
 * kernel as initramfs.
 */
class CPIO_newc {

        unsigned long m_size;

    public:
        using Member = std::shared_ptr<CPIOMember>;

        CPIO_newc()
            : m_size(0)
        { }

        /**
         * Add a new member to the archive.
         *
         * @params[in] member New member pointer
         * @returns @c true if the member was inserted,
         *          @c false if the given path was already in the archive
         */
        bool add(Member &&member);

        /**
         * Add a new member with full path. This works just like add(),
         * but it also adds all missing path components.
         *
         * @params[in] member New member pointer
         * @returns @c true if the member was inserted,
         *          @c false if the given path was already in the archive
         */
        bool addPath(Member &&member);

        void write(std::ostream &os);

    protected:
        void writeMember(std::ostream &os, CPIOMember const &member);

        std::map<std::string, Member> m_members;
};

//}}}

#endif /* CPIO_H */

// vim: set sw=4 ts=4 fdm=marker et: :collapseFolds=1:
