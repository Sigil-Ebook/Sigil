#pragma once
#ifndef PLUGINWIDGET_H
#define PLUGINWIDGET_H

#include "PreferencesWidget.h"
#include <QtGui/QStandardItemModel>

#include "ui_PPluginWidget.h"

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
    void AutoFindLua();
    void SetPy2();
    void SetPy3();
    void SetLua();
    void enginePy2PathChanged();
    void enginePy3PathChanged();
    void engineLuaPathChanged();
    void removePlugin();
    void removeAllPlugins();
    void pluginSelected(int row, int col);

private:
    enum PluginFields {
        NameField        = 0,
        VersionField     = 1,
        AuthorField      = 2,
        TypeField        = 3,
        DescriptionField = 4,
        EngineField      = 5,
        OSListField      = 6
    };

    void readSettings();
    void connectSignalsToSlots();

    Ui::PluginWidget ui;
    bool m_isDirty;
};

#endif // PLUGINWIDGET_H
