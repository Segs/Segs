
FIND_PACKAGE(wxWidgets REQUIRED base core)

IF(wxWidgets_FOUND)
    ADD_SUBDIRECTORY(wxWidgetsBased)
ENDIF(wxWidgets_FOUND)
ADD_SUBDIRECTORY(ConsoleBased)
