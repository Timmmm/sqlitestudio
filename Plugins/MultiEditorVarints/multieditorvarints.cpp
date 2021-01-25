#include "multieditorvarints.h"
#include "common/unused.h"
#include <QPlainTextEdit>
#include <QVariant>
#include <QVBoxLayout>
#include <QAction>
#include <QMenu>
#include <QDebug>
#include <QToolBar>

class RleDecoder {
    enum class State {
        Instruction,
        Copy,
        Repeat,
    } state = State::Instruction;

    // Number of values remaining for a Copy, or repeats remaining for a Repeat.
    std::uint64_t n = 0;

public:
    void operator()(std::uint64_t d, const std::function<void(std::uint64_t)>& processValue) {
        switch (state) {
            case State::Instruction: {
                bool is_copy = (d & 1) != 0;
                std::uint64_t length = d >> 1;
                n = length;
                if (is_copy) {
                    state = State::Copy;
                } else {
                    state = State::Repeat;
                }
                break;
            }
            case State::Copy: {
                processValue(d);
                if (n > 1) {
                    --n;
                } else {
                    state = State::Instruction;
                }
                break;
            }
            case State::Repeat: {
                for (std::uint64_t i = 0; i < n; ++i) {
                    processValue(d);
                }
                state = State::Instruction;
                break;
            }
        }
    }
};

namespace
{
    std::int64_t decodeZigzag(std::uint64_t x) {
        return (x >> 1) ^ (-(x & 1));
    }

    void decodeVarints(const QByteArray& ba, const std::function<void(std::uint64_t)>& processValue)
    {
        std::uint64_t value = 0;
        for (auto byte : ba) {
            value = value << 7 | (byte & 0x7f);
            if ((byte & 0x80) != 0) {
                processValue(value);
                value = 0;
            }
        }
    }
}



MultiEditorVarints::MultiEditorVarints(QWidget* parent) : MultiEditorWidget(parent)
{
    setLayout(new QVBoxLayout());

    QToolBar* tb = new QToolBar();
    rleAction = tb->addAction(tr("RLE"), this, SLOT(updateText()));
    zigzagAction = tb->addAction(tr("Zigzag"), this, SLOT(updateText()));
    rleAction->setCheckable(true);
    zigzagAction->setCheckable(true);
    layout()->addWidget(tb);

    textEdit = new QPlainTextEdit();
    textEdit->setReadOnly(true); // Modification not supported yet.
    layout()->addWidget(textEdit);
}

void MultiEditorVarints::setValue(const QVariant &value)
{
    varintsData = value.toByteArray();
    updateText();
}

void MultiEditorVarints::updateText() {
    QString text;
    text.reserve(varintsData.size());

    bool rle = rleAction->isChecked();
    bool zigzag = zigzagAction->isChecked();

    if (rle) {
        RleDecoder rleDecoder;
        decodeVarints(varintsData, [&](std::uint64_t rleVal) {
            rleDecoder(rleVal, [&](std::uint64_t val) {
                text += " ";
                if (zigzag) {
                    text += QString::number(decodeZigzag(val));
                } else {
                    text += QString::number(val);
                }
            });
        });
    } else {
        decodeVarints(varintsData, [&](std::uint64_t val) {
            text += " ";
            if (zigzag) {
                text += QString::number(decodeZigzag(val));
            } else {
                text += QString::number(val);
            }
        });
    }

    textEdit->setPlainText(text);
}

QVariant MultiEditorVarints::getValue()
{
    return varintsData;
}

void MultiEditorVarints::setReadOnly(bool value)
{
    UNUSED(value);
}

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
