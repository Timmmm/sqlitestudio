#ifndef MULTIEDITORVARINTS_H
#define MULTIEDITORVARINTS_H

#include "multieditor/multieditorwidget.h"
#include "multieditor/multieditorwidgetplugin.h"
#include "multieditorvarints_global.h"
#include "datatype.h"
#include "plugins/genericplugin.h"
#include <QVariant>

class QPlainTextEdit;
class QCheckBox;
class QLabel;

class MultiEditorVarints : public MultiEditorWidget
{
    Q_OBJECT

    public:
        MultiEditorVarints(QWidget* parent = nullptr);

        void setValue(const QVariant& value);
        QVariant getValue();
        void setReadOnly(bool value);
        QList<QWidget*> getNoScrollWidgets();
        void focusThisWidget();
        void notifyAboutUnload();

    private:
        QByteArray varintsData;
        QPlainTextEdit* textEdit = nullptr;
        QCheckBox* rleCheckbox = nullptr;
        QCheckBox* zigzagCheckbox = nullptr;
        QLabel* arrayLengthLabel = nullptr;

    private slots:
        void updateText();
};

class MULTIEDITORVARINTS_EXPORT MultiEditorVarintsPlugin : public GenericPlugin, public MultiEditorWidgetPlugin
{
    Q_OBJECT

    SQLITESTUDIO_PLUGIN("multieditorvarints.json")

    public:
        MultiEditorWidget* getInstance();
        bool validFor(const DataType& dataType);
        int getPriority(const DataType& dataType);
        QString getTabLabel();
        bool init();
        void deinit();

    private:
        QList<MultiEditorVarints*> instances;
};

#endif // MULTIEDITORVARINTS_H
