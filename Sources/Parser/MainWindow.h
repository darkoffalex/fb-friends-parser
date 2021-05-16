#pragma once

#include <QFrame>
#include <QMessageBox>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * \brief Класс основного окна приложения
 */
class MainWindow : public QWidget
{
Q_OBJECT

public:
    /**
     * \brief Конструктор основного окна
     * \param parent Указатель на родительский виджет
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * \brief Деструктор
     */
    ~MainWindow() override;

private slots:
    /**
     * \brief Обработка события клика по кнопке подключения
     */
    void on_btnConnect_clicked();

    /**
     * \brief Обработка события чтения ответа на первый запрос (инициализации)
     */
    void onInitialReplyReadyRead();

    /**
     * \brief Ошибка получения ответа на запрос инициализации
     */
    void onInitialReplyError(QNetworkReply::NetworkError code);

private:
    /// Указатель на UI объект
    Ui::MainWindow* ui_;
    /// Объект для работы HTTP запросами
    QNetworkAccessManager* httpManager_;
    /// Ответ на первый запрос (инициализации)
    QNetworkReply* replyInitial_;
    /// Ответ на запрос парсинга (используется многократно)
    QNetworkReply* replyRegular_;
};
