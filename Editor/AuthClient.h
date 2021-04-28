#pragma once

#include "core/object.h"

class AuthClient : public Object
{
    GDCLASS(AuthClient,Object)
protected:
    static AuthClient *singleton;

public:
    static AuthClient *get_singleton();

    AuthClient();
    ~AuthClient() override;
};
