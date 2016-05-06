#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDomDocument>
#include <QFile>

#include <QMessageBox>

#include "createpagedialog.h"

#include <QDebug>

#include <qjson/serializer.h>

#include <QInputDialog>
#include <QFileDialog>
#include <QColorDialog>

#include <QUrl>

#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->treeView,
            SIGNAL(doubleClicked(const QModelIndex&)),
            SLOT(slotDoubleClicked(const QModelIndex&)));

    QSettings settings("settings.ini",
                       QSettings::IniFormat);

    m_configuration.column_width_key =
            settings.value("ColumnWidthKey", 100).toInt();
    m_configuration.column_width_value =
            settings.value("ColumnWidthValue", 100).toInt();
    m_configuration.last_opened_dir_save =
            settings.value("LastOpenedDirSave", QDir::homePath()).toString();
    m_configuration.last_opened_dir_load =
            settings.value("LastOpenedDirLoad", QDir::homePath()).toString();
    m_configuration.last_opened_dir_icon =
            settings.value("LastOpenedDirIcon", QDir::homePath()).toString();
    m_configuration.last_opened_dir_file =
            settings.value("LastOpenedDirFile", QDir::homePath()).toString();

    m_model = 0;
    m_pages_root_item = 0;

    connect(ui->actionNew_page,
            SIGNAL(triggered(bool)),
            SLOT(slotTriggered(bool)));

    connect(ui->actionPublish,
            SIGNAL(triggered(bool)),
            SLOT(slotReleasePublish(bool)));

    connect(ui->actionSave,
            SIGNAL(triggered(bool)),
            SLOT(slotSave(bool)));

    connect(ui->actionLoad,
            SIGNAL(triggered(bool)),
            SLOT(slotLoad(bool)));

    connect(ui->actionNew,
            SIGNAL(triggered(bool)),
            SLOT(slotNewProject(bool)));

    m_page_counter = 0;
}

MainWindow::~MainWindow()
{
    clearModel();

    QSettings settings("settings.ini",
                       QSettings::IniFormat);

    settings.setValue("ColumnWidthKey", m_configuration.column_width_key);
    settings.setValue("ColumnWidthValue", m_configuration.column_width_value);
    settings.setValue("LastOpenedDirSave", m_configuration.last_opened_dir_save);
    settings.setValue("LastOpenedDirLoad", m_configuration.last_opened_dir_load);
    settings.setValue("LastOpenedDirIcon", m_configuration.last_opened_dir_icon);
    settings.setValue("LastOpenedDirFile", m_configuration.last_opened_dir_file);

    delete ui;
}

void MainWindow::init()
{

    QFile file(":/test_data.xml");

    if(file.open(QIODevice::ReadOnly))
    {
        QDomDocument doc;

        if(!doc.setContent(file.readAll()))
            QMessageBox::critical(this,
                                  tr("critical"),
                                  tr("Bad format!"));

        QDomNode root_node = doc.firstChild().firstChild();

        if(!root_node.isNull())
        {
            while (!root_node.isNull()) {

                if(root_node.toElement().tagName() == "modules")
                {
                    QDomNode module_node = root_node.firstChild();

                    while (!module_node.isNull()) {

                        if(module_node.toElement().tagName() == "module")
                        {
                            Module tmp_mod;

                            QDomNode module_data = module_node.firstChild();

                            while (!module_data.isNull()) {
                                module_data.toElement().tagName();
                                if(module_data.toElement().tagName() ==
                                        "tag")
                                {
                                    tmp_mod.tag = module_data.toElement().text();
                                }
                                else if(module_data.toElement().tagName() ==
                                        "title")
                                {
                                    tmp_mod.title = module_data.toElement().text();
                                }
                                else if(module_data.toElement().tagName() ==
                                        "description")
                                {
                                    tmp_mod.description = module_data.toElement().text();
                                }
                                else if(module_data.toElement().tagName() ==
                                        "parameters")
                                {
                                    QDomNode param_node = module_data.firstChild();

                                    while(!param_node.isNull())
                                    {
                                        ModuleParam module_param;

                                        module_param.name =
                                                param_node.toElement().attribute("name");

                                        QString type =
                                                param_node.toElement().attribute("type");

                                        //                                TypeString,
                                        //                                TypeReal,
                                        //                                TypeInt,
                                        //                                TypeUrl,
                                        //                                TypeColor,
                                        //                                TypeAny

                                        if(type == "string")
                                            module_param.type =
                                                    ModuleParam::TypeString;
                                        else if(type == "real")
                                            module_param.type =
                                                    ModuleParam::TypeReal;
                                        else if(type == "int")
                                            module_param.type =
                                                    ModuleParam::TypeInt;
                                        else if(type == "url")
                                            module_param.type =
                                                    ModuleParam::TypeUrl;
                                        else if(type == "color")
                                            module_param.type =
                                                    ModuleParam::TypeColor;
                                        else if(type == "file")
                                            module_param.type =
                                                    ModuleParam::TypeFile;
                                        else if(type == "any" || type == "")
                                            module_param.type =
                                                    ModuleParam::TypeAny;

                                        tmp_mod.parameters << module_param;

                                        param_node = param_node.nextSibling();
                                    }
                                }


                                module_data = module_data.nextSibling();
                            }
                            m_modules_map.insert(tmp_mod.tag, tmp_mod);

                            module_node = module_node.nextSibling();
                        }


                    }
                }
                root_node = root_node.nextSibling();
            }
        }
        else
        {
            QMessageBox::critical(this,
                                  tr("Critical!"),
                                  tr("Bad xml file!"));
        }

    }
    else
    {
        QMessageBox::critical(this,
                              tr("Critical!"),
                              tr("Cant open file!"));
    }
}

void MainWindow::createPage(PageCreator creator)
{
    QStandardItem *page_item = new QStandardItem(creator.title);
    page_item->setFlags(page_item->flags() & Qt::ItemIsEnabled);

    if(!creator.icon_path.isEmpty())
    {
        QUrl url(creator.icon_path);
        if(url.isValid())
        {
            QString icon_path = url.toLocalFile();
            page_item->setIcon(QIcon(icon_path));
            page_item->setData(icon_path,
                               RolePageIconPath);
        }
    }

    page_item->setData(creator.id, RolePageId);

    Module tmp_mod = m_modules_map.value(creator.module_tag);

    QStandardItem *module_item = new QStandardItem(tmp_mod.title);
    module_item->setData(tmp_mod.tag, RoleModuleTag);
    module_item->setData(tmp_mod.description, Qt::ToolTipRole);
    module_item->setFlags(module_item->flags() & Qt::ItemIsEnabled);

    foreach (ModuleParam param, tmp_mod.parameters) {
        QStandardItem *module_key = new QStandardItem(param.name);
        module_key->setFlags(module_key->flags() & Qt::ItemIsEnabled);

        QStandardItem *module_value = new QStandardItem();
        module_value->setData(param.type,
                              RoleParamType);

        // TODO: fail
        module_value->setData(creator.module_param.value(param.name).disp_role,
                              Qt::DisplayRole);
        module_value->setData(creator.module_param.value(param.name).data_role,
                              RoleItemData);

        int row = module_item->rowCount();
        module_item->setChild(row,
                              0,
                              module_key);

        module_item->setChild(row,
                              1,
                              module_value);

    }

    page_item->appendRow(module_item);

    m_pages_root_item->appendRow(page_item);
}

void MainWindow::slotTriggered(bool)
{
    if(m_pages_root_item)
    {
        bool ok;
        CreatePageDialog::PageParam pp =
                CreatePageDialog::getCreatePageParam(m_modules_map.keys(),
                                                     ok);

        if(ok)
        {
            PageCreator creator;

            creator.title = pp.title;
            creator.module_tag = pp.module_tag;
            creator.icon_path = pp.icon_path;
            creator.id = QString::number(m_page_counter);
            m_page_counter++;

            createPage(creator);
        }
    }
}

void MainWindow::publishPrivate(bool is_local)
{
    QString project_dir =
            QFileDialog::getExistingDirectory(this,
                                              tr("Get project directory"),
                                              QDir::homePath()); // TODO

//    bool ok;
//    QString proj_dir =
//    QInputDialog::getText(this,
//                          tr("Get save project name"),
//                          tr("Project:"),
//                          QLineEdit::Normal,
//                          tr("Project"),
//                          &ok);

    if(!project_dir.isEmpty())
    {
        QDomDocument doc;

        QDomElement rcc_elem = doc.createElement("RCC");

        QDomElement qres_elem = doc.createElement("qresource");
        qres_elem.setAttribute("prefix", "/modules");

        QDomElement qres_not_prefix_elem = doc.createElement("qresource");
        qres_not_prefix_elem.setAttribute("prefix", "/");

        QDir dir(project_dir);


        dir.mkdir("pages");

        QVariant result;
        QVariantMap result_map;

        QVariantMap meta_map;

        QVariantList pages_list;
        QVariantMap page_map;

        for(int i = 0;
            i < m_pages_root_item->rowCount();
            i++)
        {
            QStandardItem *page_item = m_pages_root_item->child(i);

            page_map.insert("id",
                            page_item->data(RolePageId));

            page_map.insert("title",
                            page_item->data(Qt::DisplayRole));

            QString icon_path;

            if(is_local)
            {
                icon_path = QUrl::fromLocalFile(
                            page_item->data(RolePageIconPath).toString()).toString();

            }
            else
            {
                QString src_path =
                        QUrl(page_item->data(RolePageIconPath).toString()).toLocalFile();

                QString dest_path;

                icon_path =
                        QString("pages") +
                        QDir::separator() +
                        page_item->data(RolePageId).toString();

                dir.mkpath(icon_path);

                icon_path +=
                        QDir::separator() +
                        QFileInfo(src_path).fileName();

                dest_path =
                        dir.absolutePath() +
                        QDir::separator() +
                        icon_path;

                QFile::copy(src_path, dest_path);

            }

            QDomElement file_elem = doc.createElement("file");
            QDomText file_text = doc.createTextNode(icon_path);

            file_elem.appendChild(file_text);
            qres_not_prefix_elem.appendChild(file_elem);


            page_map.insert("icon", icon_path);

            for(int j = 0;
                j < page_item->rowCount();
                j++)
            {
                QStandardItem *module_item = page_item->child(j);

                page_map.insert("module",
                                module_item->data(RoleModuleTag).toString());


                QVariantMap prop_map;
                for(int k = 0;
                    k < module_item->rowCount();
                    k++)
                {
                    QStandardItem* prop_key = module_item->child(k);
                    QStandardItem* prop_value = module_item->child(k, 1);


                    if(is_local)
                    {
                        prop_map.insert(prop_key->data(Qt::DisplayRole).toString(),
                                        prop_value->data(RoleItemData).toString());
                    }
                    else
                    {
                        if(prop_value->data(RoleParamType).toInt() ==
                                ModuleParam::TypeFile) {
                            QString src_path =
                                    QUrl(prop_value->data(RoleItemData).toString())
                                         .toLocalFile();
                            QString dest_path;

                            QString relative_path =
                                    QString("pages") +
                                    QDir::separator() +
                                    page_item->data(RolePageId).toString() +
                                    QDir::separator() +
                                    module_item->data(RoleModuleTag).toString();

                            dir.mkpath(relative_path);

                            relative_path += QDir::separator() +
                                    QFileInfo(src_path).fileName();

                            dest_path =
                                    dir.absolutePath() +
                                    QDir::separator() +
                                    relative_path;

                            QFile::copy(src_path, dest_path);

                            prop_map.insert(prop_key->data(Qt::DisplayRole).toString(),
                                            relative_path);

                            // TODO: fail
                            QDomElement file_elem = doc.createElement("file");
                            QDomText file_text = doc.createTextNode(relative_path);

                            file_elem.appendChild(file_text);
                            qres_elem.appendChild(file_elem);
                        }
                        else
                        {
                            prop_map.insert(prop_key->data(Qt::DisplayRole).toString(),
                                            prop_value->data(RoleItemData).toString());
                        }
                    }


                }

                page_map.insert("module_local_data", prop_map);
            }
            pages_list << page_map;
        }

        result_map.insert("meta", meta_map);
        result_map.insert("pages", pages_list);

        result = result_map;

        bool ok;

        QByteArray ba = QJson::Serializer().serialize(result, &ok);

        if(ok)
        {
            qDebug() << ba;

            QString schema_name = "schema.json";
                QString save_file =
                        dir.absolutePath() +
                        QDir::separator() +
                        schema_name;

                QFile file(save_file);

                if(file.open(QIODevice::WriteOnly))
                {
                    file.write(ba);

                    QDomElement file_elem = doc.createElement("file");
                    QDomText file_text = doc.createTextNode(schema_name);

                    file_elem.appendChild(file_text);
                    qres_not_prefix_elem.appendChild(file_elem);

                }
                else
                {
                    QMessageBox::critical(this,
                                          tr("Critical!"),
                                          tr("Can't save file %1!").arg(save_file));
                }

                rcc_elem.appendChild(qres_not_prefix_elem);
                rcc_elem.appendChild(qres_elem);

                doc.appendChild(rcc_elem);

                QString save_rcc_file =
                        dir.absolutePath() +
                        QDir::separator() +
                        "res.qrc";

                QFile rcc_file(save_rcc_file);
                if(rcc_file.open(QIODevice::WriteOnly))
                {
                    rcc_file.write(doc.toByteArray());
                }
                else
                {

                }
        }
    }
}

void MainWindow::slotReleasePublish(bool)
{
    publishPrivate(false);
}

void MainWindow::slotLocalPublish(bool)
{
}

void MainWindow::slotDoubleClicked(const QModelIndex& index)
{
    QVariant role;

    role = index.data(RoleParamType);

    if(role.isValid())
    {

        ModuleParam::ModuleParamType type =
                (ModuleParam::ModuleParamType)role.toInt();

        QVariant disp_role, data_role;

        bool ok;
        QVariant data;

        switch (type) {
        case ModuleParam::TypeString:

            data = QInputDialog::getText(this,
                                         tr("Input text"),
                                         tr("Text:"),
                                         QLineEdit::Normal,
                                         "",
                                         &ok);

            if(ok)
                disp_role = data_role = data;
            break;
        case ModuleParam::TypeReal:
            data = QInputDialog::getDouble(this,
                                           tr("Input real value"),
                                           tr("Value:"),
                                           0,
                                           -2147483647,
                                           2147483647,
                                           1,
                                           &ok);

            if(ok)
                disp_role = data_role = data;

            break;
        case ModuleParam::TypeInt:
            data = QInputDialog::getInt(this,
                                        tr("Input integer value"),
                                        tr("Value:"),
                                        0,
                                        -2147483647,
                                        2147483647,
                                        1,
                                        &ok);

            if(ok)
                disp_role = data_role = data;

            break;
        case ModuleParam::TypeUrl:
            data = QInputDialog::getText(this,
                                         tr("Input URL"),
                                         tr("URL:"),
                                         QLineEdit::Normal,
                                         "",
                                         &ok);

            if(ok)
                disp_role = data_role = data;
            break;
        case ModuleParam::TypeFile:
        {
            QString file_path = QFileDialog::getOpenFileName(this,
                                                             tr("Open file"),
                                                             QDir::homePath());

            if(!file_path.isEmpty())
            {
                disp_role = file_path;
                data_role = QUrl::fromLocalFile(file_path);
            }
        }
            break;
        case ModuleParam::TypeColor:
        {
            QColor color = QColorDialog::getColor();

            if(color.isValid())
            {
                disp_role = data_role = color.name();
            }
        }
            break;
        case ModuleParam::TypeAny:
            break;
        default:
            break;
        }

        QStandardItem* tmp_item = m_model->itemFromIndex(index);

        if(tmp_item)
        {
            tmp_item->setData(disp_role, Qt::DisplayRole);
            tmp_item->setData(data_role, RoleItemData);
        }
    }
}

void MainWindow::slotSave(bool)
{
    if(m_pages_root_item)
    {
        QDomDocument doc;

        QDomElement project_elem = doc.createElement("project");
        QDomElement pages_elem = doc.createElement("pages");


        for (int i = 0; i < m_pages_root_item->rowCount(); ++i) {
            QStandardItem *page_item = m_pages_root_item->child(i);

            QDomElement page_elem = doc.createElement("page");
            page_elem.setAttribute("id", page_item->data(RolePageId).toString());

            QDomElement page_title_elem = doc.createElement("title");
            QDomText page_title_text = doc.createTextNode(page_item->data(Qt::DisplayRole).toString());
            page_title_elem.appendChild(page_title_text);

            QDomElement page_icon_elem = doc.createElement("icon_path");
            QDomText page_icon_text = doc.createTextNode(
                        QUrl::fromLocalFile(page_item->data(RolePageIconPath).toString()).toString());

            page_icon_elem.appendChild(page_icon_text);

            page_elem.appendChild(page_title_elem);
            page_elem.appendChild(page_icon_elem);

            for (int j = 0; j < page_item->rowCount(); ++j) {
                QStandardItem *module_item = page_item->child(j);

                QDomElement module_elem = doc.createElement("module");
                module_elem.setAttribute("tag", module_item->data(RoleModuleTag).toString());

                for (int k = 0; k < module_item->rowCount(); ++k) {
                    QStandardItem *module_item_param_key = module_item->child(k);
                    QStandardItem *module_item_param_val = module_item->child(k, 1);

                    QDomElement module_param_elem = doc.createElement("param");

                    QDomText module_param_text = doc.createTextNode(module_item_param_val->data(RoleItemData).toString());

                    module_param_elem.setAttribute("name",
                                                   module_item_param_key->data(Qt::DisplayRole).toString());
                    module_param_elem.appendChild(module_param_text);

                    module_elem.appendChild(module_param_elem);
                }

                page_elem.appendChild(module_elem);
            }

            pages_elem.appendChild(page_elem);
        }

        project_elem.appendChild(pages_elem);
        doc.appendChild(project_elem);


        QString save_file = QFileDialog::getSaveFileName(this,
                                                         tr("Save project"),
                                                         m_configuration.last_opened_dir_save);

        if(!save_file.isEmpty())
        {
            QByteArray ba = doc.toByteArray();
            qDebug() << ba;

            QFileInfo fi(save_file);
            m_configuration.last_opened_dir_save = fi.absolutePath();

            QFile file(save_file);

            if(file.open(QIODevice::WriteOnly))
            {
                file.write(ba);
            }
            else
            {
                QMessageBox::critical(this,
                                      tr("Critical!"),
                                      tr("Cant save file %1!").arg(save_file));
            }
        }
    }
}

void MainWindow::slotLoad(bool)
{
    QString open_file = QFileDialog::getOpenFileName(this,
                                                     tr("Open existing project"),
                                                     m_configuration.last_opened_dir_load);

    if(!open_file.isEmpty())
    {
        QFileInfo fi(open_file);
        m_configuration.last_opened_dir_load = fi.absolutePath();

        QFile file(open_file);

        if(file.open(QIODevice::ReadOnly))
        {
            clearModel();
            createModel();

            QDomDocument doc;

            doc.setContent(file.readAll());

            QDomNode root_node = doc.firstChild().firstChild();

            while (!root_node.isNull()) {

                if(root_node.toElement().tagName() == "pages")
                {
                    QDomNode page_node = root_node.firstChild();

                    while (!page_node.isNull()) {

                        PageCreator creator;

                        if(page_node.toElement().tagName() == "page")
                        {
                            creator.id =
                                    page_node.toElement().attribute("id");

                            QDomNode module_node = page_node.firstChild();

                            while (!module_node.isNull()) {

                                if(module_node.toElement().tagName() == "title")
                                {
                                    creator.title =
                                            module_node.toElement().text();
                                }
                                else if(module_node.toElement().tagName() == "icon_path")
                                {
                                    creator.icon_path =
                                            module_node.toElement().text();
                                }
                                else if(module_node.toElement().tagName() == "module")
                                {
                                    QDomNode module_param_node = module_node.firstChild();

                                    creator.module_tag = module_node.toElement().attribute("tag");

                                    PageCreator::CreatorParamValue val;

                                    while (!module_param_node.isNull()) {

                                        if(module_param_node.toElement().tagName() == "param")
                                        {
                                            val.data_role =
                                                    val.disp_role =
                                                    module_param_node.toElement().text();

                                            creator.module_param.insert(
                                                        module_param_node.toElement().attribute("name"),
                                                        val);
                                        }

                                        module_param_node = module_param_node.nextSibling();
                                    }
                                }

                                module_node = module_node.nextSibling();
                            }
                        }

                        createPage(creator);

                        page_node = page_node.nextSibling();
                    }
                }

                root_node = root_node.nextSibling();
            }
        }
    }
}

void MainWindow::slotNewProject(bool)
{
    clearModel();
    createModel();
}

void MainWindow::clearModel()
{
    if(m_model)
    {
        m_page_counter = 0;
        m_pages_root_item = 0;

        m_configuration.column_width_key = ui->treeView->columnWidth(0);
        m_configuration.column_width_value = ui->treeView->columnWidth(1);

        ui->treeView->setModel(0);

        delete m_model;
    }
}

void MainWindow::createModel()
{
    m_model = new QStandardItemModel;

    m_model->setColumnCount(2);
    m_model->setHeaderData(0, Qt::Horizontal, tr("Key"));
    m_model->setHeaderData(1, Qt::Horizontal, tr("Value"));



    m_pages_root_item = new QStandardItem(tr("Pages"));
    m_model->insertRow(0, m_pages_root_item);

    m_pages_root_item->setFlags(m_pages_root_item->flags() & Qt::ItemIsEnabled);

    ui->treeView->setModel(m_model);

    ui->treeView->setColumnWidth(0, m_configuration.column_width_key);
    ui->treeView->setColumnWidth(1, m_configuration.column_width_value);
}
