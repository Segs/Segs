#pragma once
#include <stdint.h>
#include <EASTL/functional.h>
#include <magic_enum/magic_enum.hpp>
#include "Common/Containers/StringView.h"
#include "Common/Containers/Vector.h"

namespace SEGS {
struct IFile {
    enum OpenMode {
        None = 0x0000,
        ReadOnly = 0x0001,
        WriteOnly = 0x0002,
        ReadWrite = ReadOnly | WriteOnly,
        Append = 0x0004,
        Truncate = 0x0008,
        Text = 0x0010,
        Unbuffered = 0x0020
    };

    virtual ~IFile() = default;
    virtual bool seek(int64_t pos) = 0;
    virtual int64_t pos() const = 0;
    virtual int64_t size() const = 0;
    virtual bool atEnd() const = 0;
    virtual bool reset() = 0;
    virtual bool flush() = 0;
    virtual int64_t read(char *data, int64_t maxlen) = 0;
    virtual int64_t write(const char *data, int64_t len) = 0;
    virtual bool isSequential() const = 0;
    virtual bool open(OpenMode mode) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;

    Vector<char> readAll() {
        Vector<char> res;
        int64_t maxlen = size();
        for (int64_t read_bytes = 0; read_bytes < maxlen; read_bytes += 1024) {
            int64_t to_read = maxlen - read_bytes;
            if (to_read > 1024)
                to_read = 1024;
            Vector<char> chunk(to_read);
            int64_t chunk_read = read(chunk.data(), to_read);
            if (chunk_read <= 0)
                break;
            res.insert(res.end(), chunk.begin(), chunk.begin() + chunk_read);
        }
        return res;
    }
};

class ICompressionService {
public:
    struct DecompressionResult {
        char *data;
        uint32_t size;
        ~DecompressionResult() { delete[] data; }
    };
    struct CompressionResult {
        char *data;
        uint32_t size;
        ~CompressionResult() { delete[] data; }
    };
    virtual DecompressionResult *uncompressZip(const char *compressed_data,int compressed_size,int decompressed_size)=0;
    // compress
    virtual CompressionResult *compressData(const char *data,int data_size)=0;
};

struct FileStats {
    uint8_t exists:1=false;
    uint8_t is_dir:1=false;
    int64_t size;
    int64_t last_modified;
};

// A simple file access wrapper to allow re-locating/packing files
struct IFilesystem
{
    enum VisitResult {
        VisitNext=0,
        VisitSubdirectory=1,
        VisitStop=2
    };

    virtual ~IFilesystem() = default;

    virtual IFile* open(const char *path,int path_len, IFile::OpenMode mode)=0;
    virtual FileStats stat(StringView path)=0;
    virtual bool exists(const char *path,int path_len)=0;
    virtual void visitEntries(StringView path, eastl::function<VisitResult(StringView, bool /*is_dir*/)> visitor) = 0;
    virtual bool mkpath(StringView path) = 0;
    IFile *open(StringView path, IFile::OpenMode mode=IFile::ReadOnly) { return open(path.data(), (int)path.size(), mode); }
    bool exists(StringView path) { return exists(path.data(), (int)path.size()); }
};

struct ILogger {
    enum LogLevel {
        Debug=0,
        Info=1,
        Warning=2,
        Error=3,
        Unknown=4
    };
    virtual void logString(int log_level,const char *debug_msg)=0;
};

class IServiceLocator {
public:
    virtual ICompressionService *getCompression()=0;
    virtual IFilesystem *getFS()=0;
    virtual ILogger *getLogger()=0;
};

IServiceLocator *getServiceLocator();
void setServiceLocator(IServiceLocator *sl);

inline ICompressionService *getCompressionService() {
    IServiceLocator * sl = getServiceLocator();
    return sl ? sl->getCompression() : nullptr;
}

}
