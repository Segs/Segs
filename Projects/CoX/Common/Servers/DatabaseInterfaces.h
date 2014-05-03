#pragma once

#include <string>
#include <stdint.h>
struct PreparedArgs;
class Database;
class DbResults;
class IResultRow {
public:
    typedef std::vector<uint8_t> vBinData;
    virtual                 ~IResultRow() {}
    virtual const char *    getColString(const char *column_name)=0;
    virtual int16_t         getColInt16(const char *column_name)=0;
    virtual int32_t         getColInt32(const char *column_name)=0;
    virtual int64_t         getColInt64(const char *column_name)=0;
    virtual bool            getColBool(const char *column_name)=0;
    virtual float           getColFloat(const char *column_name)=0;
    virtual struct tm       getTimestamp(const char *column_name)=0;
    virtual vBinData        getColBinary(const char *column_name)=0;
};
class IResult {
public:
    virtual                 ~IResult() {}
    virtual const char *    message()=0;
    virtual size_t          num_rows()=0;
    virtual IResultRow *    getRow(size_t row)=0;
};

class IDataBaseCallbacks {
public:
    virtual void            on_connected(Database *) =0;
};
class IPreparedQuery
{
public:
    virtual bool            execute(PreparedArgs &args,DbResults &res)=0;
};
