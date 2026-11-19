#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class DbHandler;
class CustomTree;
class QTreeWidget;
class QTreeWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void slotAddCountry();
    void slotTreeContextMenu(const QPoint &pos);
    void slotCountryRightClicked(QMenu &menu, const QPoint &pos, QTreeWidgetItem *countryItem);
    void slotAuthorRightClicked(QMenu &menu, const QPoint &pos, QTreeWidgetItem *countryItem, QTreeWidgetItem *authorItem);
    void slotBookRightClicked(QMenu &menu, const QPoint &pos, QTreeWidgetItem *countryItem, QTreeWidgetItem *authorItem,
                                                                                             QTreeWidgetItem *bookItem);

private:
    void loadStyles();
    void createTreeView();
    QTreeWidgetItem *createItemTreeItem(const QString &text, const QVariant &data, QTreeWidgetItem *parent = nullptr);
    QTreeWidgetItem *createItemTreeWidget(const QString &text, const QVariant &data, QTreeWidget *parent = nullptr);

private:
    Ui::MainWindow *ui;
    DbHandler* m_db;
    CustomTree* m_tree;
    bool m_collapsed = true;
};
#endif // MAINWINDOW_H
