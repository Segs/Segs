#pragma once

#include "jcon.h"
#include "json_rpc_result.h"

#include <QVariant>

namespace jcon {

class JsonRpcSuccess : public JsonRpcResult
{
public:
    JsonRpcSuccess(QVariant result);
    virtual ~JsonRpcSuccess();

    bool isSuccess() const override { return true; }
    QVariant result() const override { return m_result; }
    QString toString() const override { return m_result.toString(); }

private:
    QVariant m_result;
};

}
