#include "EmailHandler.h"
#include "Common/Servers/HandlerLocator.h"
#include "Servers/MapServer/Events/MapEventTypes.h"
#include "EmailEvents.h"

void EmailHandler::dispatch(SEGSEvent *ev)
{
    assert(ev);
    switch(ev->type())
    {
        case EmailEventTypes::evEmailHeaderRequest:
        on_email_header_request();
        break;
        case EmailEventTypes::evEmailRead:
        on_email_read();
        break;
        case EmailEventTypes::evEmailSent:
        on_email_sent();
        break;
        case EmailEventTypes::evEmailDelete:
        on_email_delete();
        break;

        // for stuff not defined in EmailEvents
        case MapEventTypes::evEmailHeadersCmd: break;
        case MapEventTypes::evEmailMsgStatus: break;
        case MapEventTypes::evEmailReadCmd: break;
        default: break;
    }
}

void EmailHandler::on_email_header_request()
{}

void EmailHandler::on_email_read()
{}

void EmailHandler::on_email_sent()
{}

void EmailHandler::on_email_delete()
{}

EmailHandler::EmailHandler()
{
    HandlerLocator::setEmail_Handler(this);
}
