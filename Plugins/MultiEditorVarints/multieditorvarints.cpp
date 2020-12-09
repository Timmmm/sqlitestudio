#include "multieditorvarints.h"
#include "common/unused.h"
#include <QPlainTextEdit>
#include <QVariant>
#include <QVBoxLayout>
#include <QAction>
#include <QMenu>
#include <QDebug>

namespace
{
    QString decodeVarints(const QByteArray& ba, bool zigzag)
    {
        QString text;
        text.reserve(ba.size());
        std::uint64_t value = 0;
        for (auto byte : ba) {
            value = value << 7 | (byte & 0x7f);
            if ((byte & 0x80) != 0) {
                if (zigzag) {
                    value = (value >> 1) ^ (-(value & 1));
                }
                text += " ";
                text += QString::number(value);
                value = 0;
            }
        }
        return text;
    }
}



MultiEditorVarints::MultiEditorVarints(QWidget* parent) : MultiEditorWidget(parent)
{
    setLayout(new QVBoxLayout());
    textEdit = new QPlainTextEdit();
    textEdit->setReadOnly(true); // Modification not supported yet.
    layout()->addWidget(textEdit);

    // QToolBar* tb = new QToolBar();
    // tb->setOrientation(Qt::Vertical);
    // loadAction = tb->addAction(ICONS.OPEN_FILE, tr("Load from file"), this, SLOT(openFile()));
    // tb->addAction(ICONS.SAVE_FILE, tr("Store in a file"), this, SLOT(saveFile()));
    // zoomInAct = tb->addAction(ICONS.ZOOM_IN, tr("Zoom in by 25%"), this, SLOT(zoomIn()));
    // zoomOutAct = tb->addAction(ICONS.ZOOM_OUT, tr("Zoom out by 25%"), this, SLOT(zoomOut()));
    // tb->addAction(ICONS.ZOOM_RESET, tr("Reset zoom"), this, SLOT(resetZoom()));
    // layout()->addWidget(tb);
}

void MultiEditorVarints::setValue(const QVariant &value)
{
    varintsData = value.toByteArray();

    textEdit->setPlainText(decodeVarints(varintsData, false));
}

QVariant MultiEditorVarints::getValue()
{
    return varintsData;
}

void MultiEditorVarints::setReadOnly(bool value)
{
    UNUSED(value);
}

// TODO
// QToolBar* MultiEditorVarints::getToolBar(int toolbar) const
// {
//     UNUSED(toolbar);
//     return nullptr;
// }

QList<QWidget*> MultiEditorVarints::getNoScrollWidgets()
{
    // This is the list of widgets that we want to ignore mouse wheel scroll
    // events for.
    QList<QWidget*> list;
    return list;
}

void MultiEditorVarints::focusThisWidget()
{
    textEdit->setFocus();
}

void MultiEditorVarints::notifyAboutUnload()
{
    emit aboutToBeDeleted();
}

MultiEditorWidget* MultiEditorVarintsPlugin::getInstance()
{
    MultiEditorVarints* instance = new MultiEditorVarints();
    instances << instance;
    connect(instance, &QObject::destroyed, [this, instance]()
    {
       instances.removeOne(instance);
    });
    return instance;
}

bool MultiEditorVarintsPlugin::validFor(const DataType& dataType)
{
    switch (dataType.getType())
    {
        case DataType::BLOB:
        case DataType::NONE:
        case DataType::unknown:
            return true;
        case DataType::BOOLEAN:
        case DataType::BIGINT:
        case DataType::DECIMAL:
        case DataType::DOUBLE:
        case DataType::INTEGER:
        case DataType::INT:
        case DataType::NUMERIC:
        case DataType::REAL:
        case DataType::STRING:
        case DataType::TEXT:
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::DATE:
        case DataType::DATETIME:
        case DataType::TIME:
            break;
    }
    return false;
}

int MultiEditorVarintsPlugin::getPriority(const DataType& dataType)
{
    switch (dataType.getType())
    {
        case DataType::BLOB:
            return 10;
        case DataType::NONE:
        case DataType::unknown:
            return 50;
        case DataType::BOOLEAN:
        case DataType::BIGINT:
        case DataType::DECIMAL:
        case DataType::DOUBLE:
        case DataType::INTEGER:
        case DataType::INT:
        case DataType::NUMERIC:
        case DataType::REAL:
        case DataType::STRING:
        case DataType::TEXT:
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::DATE:
        case DataType::DATETIME:
        case DataType::TIME:
            break;
    }
    return 100;
}

QString MultiEditorVarintsPlugin::getTabLabel()
{
    return tr("Varints");
}

bool MultiEditorVarintsPlugin::init()
{
    Q_INIT_RESOURCE(multieditorvarints);
    return GenericPlugin::init();
}

void MultiEditorVarintsPlugin::deinit()
{
    for (MultiEditorVarints* editor : instances)
    {
        editor->notifyAboutUnload();
        delete editor;
    }

    Q_CLEANUP_RESOURCE(multieditorvarints);
}
