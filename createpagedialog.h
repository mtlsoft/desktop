#ifndef CREATEPAGEDIALOG_H
#define CREATEPAGEDIALOG_H

#include <QDialog>

namespace Ui {
class CreatePageDialog;
}

class CreatePageDialog : public QDialog
{
    Q_OBJECT

    static int m_create_page_counter;

    explicit CreatePageDialog(QStringList modules,
                              QWidget *parent = 0);
    ~CreatePageDialog();
public:

    struct PageParam
    {
        QString title;
        QString icon_path;
        QString module_tag;
    };

    static PageParam getCreatePageParam(QStringList modules,
                                        bool &ok);


private:
    Ui::CreatePageDialog *ui;
private slots:
    void slotGetIconClicked();
};

#endif // CREATEPAGEDIALOG_H
