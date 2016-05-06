#include "createpagedialog.h"
#include "ui_createpagedialog.h"

#include <QFileDialog>

int CreatePageDialog::m_create_page_counter = 0;

CreatePageDialog::CreatePageDialog(QStringList modules,
                                   QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreatePageDialog)
{
    ui->setupUi(this);

    foreach (QString module_str, modules) {
        ui->cbModule->addItem(module_str);
    }


    connect(ui->pbGetIcon,
            SIGNAL(clicked()),
            SLOT(slotGetIconClicked()));
}

CreatePageDialog::~CreatePageDialog()
{
    delete ui;
}

CreatePageDialog::PageParam
CreatePageDialog::getCreatePageParam(QStringList modules,
                                     bool &ok)
{
    PageParam ret_param;

    CreatePageDialog create_dialog(modules);

    create_dialog.ui->leTitle->setText(tr("Page") +
                                       " " +
                                       QString::number(create_dialog.m_create_page_counter));

    create_dialog.exec();

    ok = create_dialog.result() == QDialog::Accepted;

    if(ok)
    {
        ok = !create_dialog.ui->leTitle->text().isEmpty();
        if(ok)
            ret_param.title =
                    create_dialog.ui->leTitle->text();

        ok = !create_dialog.ui->lbIconPath->text().isEmpty();
//        if(ok) TODO:!!!!
            ret_param.icon_path =
                    create_dialog.ui->lbIconPath->text();

        ok = !(create_dialog.ui->cbModule->currentText() == tr("Choose module"));
        if(ok)
            ret_param.module_tag =
                    create_dialog.ui->cbModule->currentText();

        if(ok)
            create_dialog.m_create_page_counter++;
    }

    return ret_param;
}

void CreatePageDialog::slotGetIconClicked()
{
    QString icon_path = QFileDialog::getOpenFileName(this,
                                                     tr("Get icon"),
                                                     QDir::homePath());

    if(!icon_path.isEmpty())
        ui->lbIconPath->setText(icon_path);
}
