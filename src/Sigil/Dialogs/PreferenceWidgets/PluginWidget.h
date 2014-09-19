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
    void readSettings();
    void connectSignalsToSlots();

    Ui::PluginWidget ui;
    bool m_isDirty;

    QString m_PluginsPath;
    
};

#endif // PLUGINWIDGET_H
