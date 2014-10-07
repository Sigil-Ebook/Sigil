#pragma once
#ifndef PLUGINWIDGET_H
#define PLUGINWIDGET_H

#include "PreferencesWidget.h"
#include <QtGui/QStandardItemModel>

#include "ui_PPluginWidget.h"

class Plugin;

class PluginWidget : public PreferencesWidget
{
    Q_OBJECT

public:
    PluginWidget();
    PreferencesWidget::ResultAction saveSettings();

private slots:
    void addPlugin();
    void AutoFindPy2();
    void AutoFindPy3();
    void SetPy2();
    void SetPy3();
    void enginePy2PathChanged();
    void enginePy3PathChanged();
    void removePlugin();
    void removeAllPlugins();
    void pluginSelected(int row, int col);

private:
    enum PluginFields {
        NameField        = 0,
        VersionField     = 1,
        AuthorField      = 2,
        TypeField        = 3,
        EngineField      = 4,
        OSListField      = 5
    };

    void readSettings();
    void connectSignalsToSlots();
    void setPluginTableRow(Plugin *p, int row);

    Ui::PluginWidget ui;
    bool m_isDirty;
};

#endif // PLUGINWIDGET_H
