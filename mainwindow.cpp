#include <QDebug>
#include <QFile>
#include <QMessageBox>

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "customtree.h"
#include "dbhandler.h"
#include "inputdialog.h"
#include "quoteform.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->treeWidget->setHeaderLabel("Countries");
    ui->horizontalLayout_2->setStretch(0, 2);
    ui->horizontalLayout_2->setStretch(1, 3);

    ui->pb_show_hide->setIcon(QIcon(":/icons/arrow_top.png"));
    ui->pb_add_country->setIcon(QIcon(":/icons/globe.png"));

    m_db = new DbHandler();
    auto initialData = m_db->loadData();
    if (!initialData.has_value()) {
        /// добавить обработку
        return;
    }

    m_tree = new CustomTree();
    m_tree->setCountries(std::move(*initialData));

    createTreeView();

    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &MainWindow::slotTreeContextMenu);

    connect(ui->pb_add_country, &QPushButton::clicked, this, &MainWindow::slotAddCountry);
    connect(ui->pb_show_hide, &QPushButton::clicked, this, [&](){
        if (m_collapsed) { ui->treeWidget->expandAll();   ui->pb_show_hide->setIcon(QIcon(":/icons/arrow_down.png")); }
        else             { ui->treeWidget->collapseAll(); ui->pb_show_hide->setIcon(QIcon(":/icons/arrow_top.png")); }

        m_collapsed = !m_collapsed;
    });

    loadStyles();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slotAddCountry()
{
    InputDialog dlg(DialogType::Country, this);
    if (dlg.exec() != QDialog::Accepted) return;

    const QString name = dlg.name();
    if (name.isEmpty()) return;

    const AddResult res = m_tree->addCountry(name);
    if (res.status == AddStatus::AlreadyExists) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Страна «%1» уже существует.").arg(name));
        return;
    } else if (res.status != AddStatus::Added) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось добавить страну."));
        return;
    }

    const int dbId = m_db->addCountry(name);
    if (dbId < 0) {
        QMessageBox::warning(this, tr("Ошибка БД"), tr("Не удалось сохранить страну в базе."));
        m_tree->removeCountry(name);
        return;
    }

    QTreeWidgetItem *item = createItemTreeItem(name, dbId);
    if (!item) return;

    ui->treeWidget->insertTopLevelItem(res.row, item);
}

void MainWindow::slotTreeContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = ui->treeWidget->itemAt(pos);
    if (!item) return; /// пкм по пустому месту

    QTreeWidgetItem *parent = item->parent();
    QMenu menu(this);

    if (!parent) {
        QTreeWidgetItem *countryItem = item;
        slotCountryRightClicked(menu, pos, countryItem);
    }
    else if (!parent->parent()) {
        QTreeWidgetItem *countryItem = parent;
        QTreeWidgetItem *authorItem = item;

        slotAuthorRightClicked(menu, pos, countryItem, authorItem);
    }
    else if (!parent->parent()->parent()) {
        QTreeWidgetItem *countryItem = parent->parent();
        QTreeWidgetItem *authorItem = parent;
        QTreeWidgetItem *bookItem = item;

        slotBookRightClicked(menu, pos, countryItem, authorItem, bookItem);
    }
}

void MainWindow::slotCountryRightClicked(QMenu &menu, const QPoint &pos, QTreeWidgetItem *countryItem)
{
    QAction *actAddAuthor = menu.addAction(tr("Добавить автора"));
    QAction *actRemove    = menu.addAction(tr("Удалить"));

    QAction *chosen = menu.exec(ui->treeWidget->viewport()->mapToGlobal(pos));
    if (!chosen) return;

    const QString countryName = countryItem->text(0);
    const int countryId       = countryItem->data(0, Qt::UserRole).toInt();

    if      (chosen == actAddAuthor) addAuthor(countryItem, countryName, countryId);
    else if (chosen == actRemove)    removeCountry(countryItem, countryName, countryId);
}

void MainWindow::slotAuthorRightClicked(QMenu &menu, const QPoint &pos, QTreeWidgetItem *countryItem, QTreeWidgetItem *authorItem)
{
    QAction *actAddBook   = menu.addAction(tr("Добавить книгу"));
    QAction *actRemove    = menu.addAction(tr("Удалить"));

    QAction *chosen = menu.exec(ui->treeWidget->viewport()->mapToGlobal(pos));
    if (!chosen) return;

    const QString countryName = countryItem->text(0);
    const QString authorName  = authorItem->text(0);
    const int authorId        = authorItem->data(0, Qt::UserRole).toInt();

    if      (chosen == actAddBook) addBook(authorItem, countryName, authorName, authorId);
    else if (chosen == actRemove)  removeAuthor(authorItem, countryName, authorName, authorId);
}

void MainWindow::slotBookRightClicked(QMenu &menu, const QPoint &pos, QTreeWidgetItem *countryItem, QTreeWidgetItem *authorItem,
                                      QTreeWidgetItem *bookItem)
{
    QAction *actAddQuote  = menu.addAction(tr("Добавить цитату"));
    QAction *actRemove    = menu.addAction(tr("Удалить"));

    QAction *chosen = menu.exec(ui->treeWidget->viewport()->mapToGlobal(pos));
    if (!chosen) return;

    const QString countryName = countryItem->text(0);
    const QString authorName  = authorItem->text(0);
    const QString bookName    = bookItem->text(0);
    const int     bookId      = bookItem->data(0, Qt::UserRole).toInt();

    if      (chosen == actAddQuote) addQuote(countryName, authorName, bookName, bookId);
    else if (chosen == actRemove)   removeBook(bookItem, countryName, authorName, bookName, bookId);
}

void MainWindow::slotQuoteContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = ui->listWidget->itemAt(pos);
    if (!item) return;

    QMenu menu(this);
    QAction *actEdit   = menu.addAction(tr("Редактировать"));
    QAction *actRemove = menu.addAction(tr("Удалить"));

    QAction *chosen = menu.exec(ui->listWidget->viewport()->mapToGlobal(pos));
    if (!chosen) return;

    if (chosen == actEdit) {
        /// todo
    } else if (chosen == actRemove) {
        removeQuote(item);
    }
}

void MainWindow::addAuthor(QTreeWidgetItem *countryItem, const QString &countryName, const int countryId)
{
    InputDialog dlg(DialogType::Author, this);
    if (dlg.exec() != QDialog::Accepted) return;

    const QString authorName = dlg.name();
    if (authorName.isEmpty()) return;

    const AddResult res = m_tree->addAuthor(countryName, authorName);
    if (res.status == AddStatus::AlreadyExists) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Автор «%1» уже существует.").arg(authorName));
        return;
    } else if (res.status != AddStatus::Added) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось добавить автора."));
        return;
    }

    const int dbId = m_db->addAuthor(countryId, authorName);
    if (dbId < 0) {
        QMessageBox::warning(this, tr("Ошибка БД"), tr("Не удалось сохранить автора в базе."));
        m_tree->removeAuthor(countryName, authorName);
        return;
    }

    QTreeWidgetItem *authorItem = createItemTreeItem(authorName, dbId);
    if (!authorItem) return;

    countryItem->insertChild(res.row, authorItem);
    countryItem->setExpanded(true);

    ui->treeWidget->setCurrentItem(authorItem);
}

void MainWindow::removeCountry(QTreeWidgetItem *countryItem, const QString &countryName, const int countryId)
{
    const auto answer = QMessageBox::question(
        this,
        tr("Удаление страны"),
        tr("Удалить страну «%1»?\n\n"
           "Вместе с ней будут удалены все её авторы, книги и цитаты. "
           "Действие необратимо.")
            .arg(countryName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);   /// No по умолчанию — защита от случайного Enter

    if (answer != QMessageBox::Yes)
        return;

    if (!m_db->removeCountry(countryId)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось удалить страну из базы"));
        return;
    }

    if (!m_tree->removeCountry(countryName)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось удалить страну."));
        return;
    }

    delete countryItem;
}

void MainWindow::addBook(QTreeWidgetItem *authorItem, const QString &countryName, const QString &authorName, const int authorId)
{
    InputDialog dlg(DialogType::Book, this);
    if (dlg.exec() != QDialog::Accepted) return;

    const QString bookName = dlg.name();
    if (bookName.isEmpty()) return;

    const AddResult res = m_tree->addBook(countryName, authorName, bookName);
    if (res.status == AddStatus::AlreadyExists) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Книга «%1» уже существует.").arg(bookName));
        return;
    } else if (res.status != AddStatus::Added) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось добавить книгу."));
        return;
    }

    const int dbId = m_db->addBook(authorId, bookName);
    if (dbId < 0) {
        QMessageBox::warning(this, tr("Ошибка БД"), tr("Не удалось сохранить книгу в базе."));
        m_tree->removeBook(countryName, authorName, bookName);
        return;
    }

    QTreeWidgetItem *bookItem = createItemTreeItem(bookName, dbId);
    authorItem->insertChild(res.row, bookItem);
    authorItem->setExpanded(true);
    ui->treeWidget->setCurrentItem(bookItem);
}

void MainWindow::removeAuthor(QTreeWidgetItem *authorItem, const QString &countryName, const QString &authorName, const int authorId)
{
    const auto answer = QMessageBox::question(
        this,
        tr("Удаление автора"),
        tr("Удалить автора «%1»?\n\n"
           "Вместе с ним будут удалены все его книги и цитаты. "
           "Действие необратимо.")
            .arg(authorName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (answer != QMessageBox::Yes)
        return;

    if (!m_db->removeAuthor(authorId)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось удалить автора из базы"));
        return;
    }

    if (!m_tree->removeAuthor(countryName, authorName)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось удалить автора."));
        return;
    }

    delete authorItem;
}

void MainWindow::addQuote(const QString &countryName, const QString &authorName, const QString &bookName, const int bookId)
{
    InputDialog dlg(DialogType::Quote, this);
    if (dlg.exec() != QDialog::Accepted) return;

    const QString quote = dlg.text();
    const QString quoteAuthor = dlg.quoteAuthor();

    if (quote.isEmpty()) return;

    const AddResult res = m_tree->addQuote(countryName, authorName, bookName, quote, quoteAuthor);
    if (res.status != AddStatus::Added) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось добавить цитату."));
        return;
    }

    const int dbId = m_db->addQuote(bookId, quote, quoteAuthor);
    if (dbId < 0) {
        QMessageBox::warning(this, tr("Ошибка БД"), tr("Не удалось сохранить книгу в базе."));
        int quotesCount = ui->listWidget->count();
        m_tree->removeQuote(countryName, authorName, bookName, quotesCount);
        return;
    }

    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    QuoteForm *q          = new QuoteForm(quote, quoteAuthor);
    q->adjustSize();
    item->setSizeHint(q->sizeHint());
    item->setData(Qt::UserRole, dbId);
    item->setData(Qt::UserRole + 1, countryName);
    item->setData(Qt::UserRole + 2, authorName);
    item->setData(Qt::UserRole + 3, bookName);
    ui->listWidget->setItemWidget(item, q);
}

void MainWindow::removeBook(QTreeWidgetItem *bookItem, const QString &countryName, const QString &authorName,
                            const QString &bookName, const int bookId)
{
    const auto answer = QMessageBox::question(
        this,
        tr("Удаление книги"),
        tr("Удалить книгу «%1»?\n\n"
           "Вместе с ней будут удалены все её цитаты. "
           "Действие необратимо.")
            .arg(bookName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (answer != QMessageBox::Yes)
        return;

    if (!m_db->removeBook(bookId)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось удалить книгу из базы"));
        return;
    }

    if (!m_tree->removeBook(countryName, authorName, bookName)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось удалить книгу."));  // было «автора»
        return;
    }

    delete bookItem;
}

void MainWindow::removeQuote(QListWidgetItem *item)
{
    const int quoteId       = item->data(Qt::UserRole).toInt();
    const QString country   = item->data(Qt::UserRole + 1).toString();
    const QString author    = item->data(Qt::UserRole + 2).toString();
    const QString book      = item->data(Qt::UserRole + 3).toString();
    const int   quoteIndex  = ui->listWidget->row(item);

    if (!m_db->removeQuote(quoteId)) {
        QMessageBox::warning(this, tr("Ошибка БД"), tr("Не удалось удалить цитату из базы."));
        return;
    }

    if (!m_tree->removeQuote(country, author, book, quoteIndex)) {
        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось удалить цитату."));
        return;
    }

    delete item;
}

void MainWindow::loadStyles()
{
    QFile f(":/styles/style.qss");
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Не открылся style.qss:" << f.errorString();
        return;
    }

    const QString qss = QString::fromUtf8(f.readAll());
    setStyleSheet(qss);
}

void MainWindow::createTreeView()
{
    ui->treeWidget->clear();

    for (int i = 0; i < m_tree->getCountries().size(); ++i) {
        const Country &country = m_tree->getCountries()[i];
        QTreeWidgetItem *countryItem = createItemTreeWidget(country.name, country.id, ui->treeWidget);

        for (int j = 0; j < country.authors.size(); ++j) {
            const Author &author = country.authors[j];
            QTreeWidgetItem *authorItem = createItemTreeItem(author.name, author.id, countryItem);

            for (int k = 0; k < author.books.size(); ++k) {
                const Book &book = author.books[k];
                createItemTreeItem(book.name, book.id, authorItem);
            }
        }
    }
}

QTreeWidgetItem *MainWindow::createItemTreeItem(const QString &text, const QVariant &data, QTreeWidgetItem *parent)
{
    if (text.isEmpty()) { qDebug() << "Отсутствует текст"; return nullptr; }
    if (data.isNull())  { qDebug() << "Не передана userData"; return nullptr; }

    QTreeWidgetItem *item = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem;

    item->setText(0, text);
    item->setData(0, Qt::UserRole, data);

    return item;
}

QTreeWidgetItem *MainWindow::createItemTreeWidget(const QString &text, const QVariant &data, QTreeWidget *parent)
{
    if (text.isEmpty()) { qDebug() << "Отсутствует текст"; return nullptr; }
    if (data.isNull())  { qDebug() << "Не передана userData"; return nullptr; }

    QTreeWidgetItem *item = parent ? new QTreeWidgetItem(parent) : new QTreeWidgetItem;

    item->setText(0, text);
    item->setData(0, Qt::UserRole, data);

    return item;
}
