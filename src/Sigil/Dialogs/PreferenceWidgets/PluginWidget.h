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
    void AutoFindPy27();
    void AutoFindPy34();
    void AutoFindLua52();
    void SetPy27();
    void SetPy34();
    void SetLua52();
    void enginePy27PathChanged();
    void enginePy34PathChanged();
    void engineLua52PathChanged();
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
