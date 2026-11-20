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
class QListWidgetItem;

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
    void slotBookSelected(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void slotQuoteContextMenu(const QPoint &pos);

private:
    void addAuthor(QTreeWidgetItem *countryItem, const QString &countryName, const int countryId);
    void removeCountry(QTreeWidgetItem *countryItem, const QString &countryName, const int countryId);
    void addBook(QTreeWidgetItem *authorItem, const QString &countryName, const QString &authorName, const int authorId);
    void removeAuthor(QTreeWidgetItem *authorItem, const QString &countryName, const QString &authorName, const int authorId);
    void addQuote(const QString &countryName, const QString &authorName, const QString &bookName, const int bookId);
    void removeBook(QTreeWidgetItem *bookItem, const QString &countryName, const QString &authorName,
                                                          const QString &bookName, const int bookId);
    void removeQuote(QListWidgetItem* item);
    void editQuote(QListWidgetItem* item);

private:
    void loadStyles();
    void createTreeView();
    QTreeWidgetItem *createItemTreeItem(const QString &text, const QVariant &data, QTreeWidgetItem *parent = nullptr);
    QTreeWidgetItem *createItemTreeWidget(const QString &text, const QVariant &data, QTreeWidget *parent = nullptr);

    /// пути и настройки
    static QString defaultDbPath();    /// путь по умолчанию <exe>/quotes_app.db (абсолютный)
    static QString settingsFilePath(); /// путь по умолчанию <exe>/settings.ini  (абсолютный)
    QString loadDbPathFromSettings() const;
    void saveDbPathToSettings();
    void updateCurrentPathLabel();

private:
    Ui::MainWindow *ui;
    QString m_currentDbPath;
    DbHandler* m_db;
    CustomTree* m_tree;
    bool m_collapsed = true;
};
#endif // MAINWINDOW_H
