#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QStandardItemModel>

namespace Ui {
class MainWindow;
}


struct ModuleParam
{
    enum ModuleParamType
    {
        TypeString,
        TypeReal,
        TypeInt,
        TypeUrl,
        TypeFile,
        TypeColor,
        TypeAny
    };

  ModuleParamType type;
  QString name;

};

struct Module
{
    QString tag;
    QString title;
    QString description;

    QList <ModuleParam> parameters;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum ModelLocalRoles
    {
        RoleParamType = Qt::UserRole + 1,
        RoleItemData,

        RolePageIconPath,
        RolePageId,

        RoleModuleTag
    };

    struct
    {
        int column_width_key;
        int column_width_value;

        QString last_opened_dir_save;
        QString last_opened_dir_load;
        QString last_opened_dir_icon;
        QString last_opened_dir_file;
    } m_configuration;

    struct PageCreator
    {
        QString id;
        QString title;
        QString icon_path;

        QString module_tag;

        struct CreatorParamValue
        {
            QVariant disp_role;
            QVariant data_role;
        };

        QMap <QString, CreatorParamValue> module_param;
    };

    QStandardItemModel *m_model;

    QList<Module> m_modules;
    QMap<QString, Module> m_modules_map;

    QStandardItem *m_pages_root_item;
    int m_page_counter;

    void clearModel();
    void createModel();

    void createPage(PageCreator creator);

    void publishPrivate(bool is_local);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void init();

private slots:
    void slotTriggered(bool);
    void slotLocalPublish(bool);
    void slotReleasePublish(bool);
    void slotDoubleClicked(const QModelIndex&);

    void slotSave(bool);
    void slotLoad(bool);

    void slotNewProject(bool);
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
