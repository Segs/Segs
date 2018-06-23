#ifndef EMAILHANDLER_H
#define EMAILHANDLER_H

#include "EventProcessor.h"

class EmailHandler : public EventProcessor
{
private:
    // EventProcessor interface
    void dispatch(SEGSEvent *ev) override;

    void on_email_header_request();
    void on_email_sent();
    void on_email_read();
    void on_email_delete();
public:
    EmailHandler();
};

#endif // EMAILHANDLER_H
