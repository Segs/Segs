
#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QRegularExpression>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>
#include <QtCore/QDirIterator>
#include <QtCore/QString>

#include <QTextStream>

// event_gen is an utility that scans provided directories for tagged Event structures
// it handles the following tags:
// `// [[ev_def:type]]`  which marks that a new Event type is defined on following line
// `// [[ev_def:field]]` which marks the following structure/class field as serializable
// `// [[ev_def:macro]]` which is used to handle *_MESSAGE macros.


struct DefElement
{
    QString type;
    QString name;
};
struct DefEvent
{
    QString name;
    std::vector<DefElement> elements;
};
struct DefFile
{
    QString source_name;
    std::vector<DefEvent> events;
};

void generate_cpp(const std::vector<DefFile> &files,const QString &group_name,const QString &tgt_dir)
{
    QFile tgt_file(tgt_dir+"/" + group_name+".gen.cpp");
    tgt_file.open(QFile::WriteOnly);
    QTextStream tgt_stream(&tgt_file);
    tgt_stream << "// THIS FILE WAS GENERATED, DO NOT EDIT\n";
    if(group_name=="MapServerEvents")
        {
            tgt_stream << "#include \"Components/BitStream.h\"\n";
        }
    for(const DefFile &d : files)
    {
        tgt_stream << QString("#include \"%1\"\n\n").arg(d.source_name);
    }

    tgt_stream << QString("#include \"Components/SEGSEventFactory.h\"\n");
    tgt_stream << "#include \"Components/serialization_common.h\"\n\n";
    tgt_stream << "#include \"Components/serialization_types.h\"\n\n";
    tgt_stream << "#include <QDebug>\n\n";
    tgt_stream << "using namespace SEGSEvents;\n\n";
    for(const DefFile &d : files)
    {
        for(const DefEvent &ev : d.events)
        {
            tgt_stream << "template<class Archive>\n";
            if(ev.elements.empty())
                tgt_stream << QString("void %1::serialize(Archive &)").arg(ev.name);
            else
                tgt_stream << QString("void %1::serialize(Archive & archive)").arg(ev.name);
            tgt_stream << "{\n";
            for(const DefElement &el : ev.elements)
            {
                QString internal_field_name = el.name;
                if(internal_field_name.startsWith("m_"))
                    internal_field_name = internal_field_name.mid(2);
                tgt_stream << QString("    archive(cereal::make_nvp(\"%1\",%2));\n").arg(internal_field_name,el.name);
            }
            tgt_stream << "}\n";
            tgt_stream << QString("SPECIALIZE_SERIALIZATIONS(%1)\n").arg(ev.name);
        }
    }
    tgt_stream << "\n";
    tgt_stream << QString("void register_%1()\n").arg(group_name);
    tgt_stream << "{\n";
    for(const DefFile &d : files)
    {
        for(const DefEvent &ev : d.events)
        {
            tgt_stream << QString("register_event_type(\"%1\",ev%1,[]()->Event * { return new %1;});\n").arg(ev.name);
        }
    }
    tgt_stream << "}\n";
}

bool extract_annotated(const QString &hdr_contents,DefFile &tgt)
{
    int idx=0;
    bool in_type=false;
    DefEvent cur_event;
    QRegularExpression whitespace("\\s+");
    QRegularExpression inherits_event(R"(^(.+)\s*:\s*public\s+(\w*Event)\s*$)");
    QRegularExpression event_macro(R"(^(ONE|TWO)_WAY_MESSAGE\(\w+,(\w+)\))");
    inherits_event.setPatternOptions(QRegularExpression::MultilineOption);
    event_macro.setPatternOptions(QRegularExpression::MultilineOption);
    QStringList detected_event_classes;

    QRegularExpressionMatchIterator i = inherits_event.globalMatch(hdr_contents);
    while (i.hasNext())
    {
        QRegularExpressionMatch match = i.next();
        if(match.hasMatch())
        {
            QString event_classname = match.captured(1).trimmed().split(whitespace,QString::SkipEmptyParts)[1];
            QString parent_classname = match.captured(2).trimmed();
            detected_event_classes << event_classname;
            detected_event_classes.removeOne(parent_classname);
        }
    }
    QRegularExpressionMatchIterator iter2 = event_macro.globalMatch(hdr_contents);
    while (iter2.hasNext())
    {
        QRegularExpressionMatch match = iter2.next();
        if(match.hasMatch())
        {
            bool is_one_way = match.captured(1)=="ONE";
            QString event_classname = match.captured(2);
            if(is_one_way)
                detected_event_classes << event_classname+"Message";
            else
            {
                detected_event_classes << event_classname+"Request";
                detected_event_classes << event_classname+"Response";
            }
        }
    }
    while(true)
    {
        idx = hdr_contents.indexOf("[[ev_def:",idx+1);
        if(idx==-1)
            break;
        idx+=9;
        if(hdr_contents.midRef(idx).startsWith("type"))
        {
            idx+=4;
            if(in_type)
            {
                tgt.events.emplace_back(cur_event);
                cur_event = DefEvent(); // reset to uninitialized
            }
            in_type=true;
            idx  = hdr_contents.indexOf("\n",idx+1);
            int start=idx+1;
            idx  = hdr_contents.indexOf("{",idx+1);
            QString class_name = hdr_contents.mid(start,idx-start).split(whitespace,QString::SkipEmptyParts)[1];
            cur_event.name = class_name;
            detected_event_classes.removeOne(class_name);
        }
        else if(hdr_contents.midRef(idx).startsWith("field"))
        {
            DefElement el;
            idx  = hdr_contents.indexOf("\n",idx+1);
            int start=idx+1;
            idx  = hdr_contents.indexOf(";",idx+1);
            QStringList parts= hdr_contents.mid(start,idx-start).split(whitespace,QString::SkipEmptyParts);
            if(parts.size()<2)
                return false;
            el.type = parts[0];
            el.name = parts[1];
            if(el.name.contains('['))
            {
                el.name = el.name.mid(0,el.name.indexOf("["));
            }
            if(el.name.contains('='))
            {
                el.name = el.name.mid(0,el.name.indexOf("="));
            }
            cur_event.elements.emplace_back(el);
        }
        else if(hdr_contents.midRef(idx).startsWith("macro"))
        {
            static const QStringList handled_macros {"ONE_WAY_MESSAGE","TWO_WAY_MESSAGE"};
            DefElement el;
            idx  = hdr_contents.indexOf("\n",idx+1);
            int start=idx+1;
            idx  = hdr_contents.indexOf(")",start);
            QStringList parts= hdr_contents.mid(start,idx-start).split('(',QString::SkipEmptyParts);
            if(parts.size()<2)
                return false;
            QString classname = parts[1].split(",")[1];
            int macro_type = handled_macros.indexOf(parts[0]);
            if(macro_type==-1)
            {
                qWarning() << "Unhandled macro"<<parts[0]<<"for type"<<classname<<"has been tagged.";
                exit(-1);
            }
            if(in_type)
            {
                tgt.events.emplace_back(cur_event);
                cur_event = DefEvent(); // reset to uninitialized
            }
            switch(macro_type)
            {
                case 0: // ONE_WAY_MESSAGE
                {
                    DefEvent msg;
                    msg.name = classname+"Message";
                    msg.elements.push_back({classname+"Data","m_data"});
                    tgt.events.emplace_back(msg);
                    detected_event_classes.removeOne(msg.name);
                    break;
                }
                case 1: // TWO_WAY_MESSAGE
                {
                    DefEvent request;
                    DefEvent response;
                    request.name = classname+"Request";
                    request.elements.push_back({classname+"RequestData","m_data"});
                    tgt.events.emplace_back(request);
                    response.name = classname+"Response";
                    response.elements.push_back({classname+"ResponseData","m_data"});
                    tgt.events.emplace_back(response);
                    detected_event_classes.removeOne(request.name);
                    detected_event_classes.removeOne(response.name);
                }
                break;
            }
            in_type=false;
        }
    }
    // HACK: GameCommandEvent is alone in it's header, thus it is not detected as a base class
    detected_event_classes.removeOne("GameCommandEvent");
    detected_event_classes.removeOne("InternalEvent");
    if(in_type)
        tgt.events.emplace_back(cur_event);
    for(const QString &x : detected_event_classes)
    {
        qWarning() << "Event"<<x<<"has not been tagged.";
    }
    if(!detected_event_classes.isEmpty())
        exit(-1);
    return !tgt.events.empty();
}
bool generate_factory()
{
    return false;
}
int main(int argc,char **argv)
{
    // Modes:
    //  gen_impl Header Type tgt_dir
    //  gen_factory EventListFile tgt_dir
    QCoreApplication app(argc,argv);
    // check argument count
    if(argc<4)
        return -1;
    if(QString(argv[1])=="scan_events") // Group Tgt_Dir [Directory]+
    {
        std::vector<DefFile> extracted_annotations;
        QString group_name = argv[2];
        QString tgt_dirname = argv[3];
        QStringList dirs_to_scan = app.arguments().mid(4);
        if(dirs_to_scan.isEmpty())
        {
            qWarning() << "At least a single directory has to be provided for 'scan_events' operation";
            return -1;
        }
        for(QString s : dirs_to_scan)
        {
            qDebug() << "Scanning dir"<<s;
            QDirIterator dir_contents(s,{"*.h"},QDir::Files,QDirIterator::NoIteratorFlags);
            while(dir_contents.hasNext())
            {
                QString scan_file_name = dir_contents.next();
                QFile scan_file(scan_file_name);
                scan_file.open(QFile::ReadOnly);
                QString contents = scan_file.readAll();
                DefFile annotations;
                bool res = extract_annotated(contents,annotations);
                if(res && !annotations.events.empty())
                {
                    annotations.source_name = scan_file_name;
                    extracted_annotations.emplace_back(std::move(annotations));
                }
            }
        }
        // produce h/cpp files
        generate_cpp(extracted_annotations,group_name,tgt_dirname);
        return 0;
    }
    return -1;
}
