#pragma once

#include <QFrame>
#include <QString>
#include <QMessageBox>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QRegularExpression>

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
     * \brief Состояния приложения
     */
    enum class Status
    {
        // Остановлено / не запущено
        eStopped = 0,
        // Приостановлено
        ePaused = 1,
        // В процессе
        eParsing = 2
    };

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
     * \brief Обработчка события готовности чтения ответа
     * \details Данный сигнал может срабатывать НЕСКОЛЬКО раз при обработке ОДНОГО ответа (данные загружаются порциями)
     */
    void onReplyReadyRead();

    /**
     * \brief Обработка события ошибки ответа
     * \param code Код ошибки
     */
    void onReplyError(QNetworkReply::NetworkError code);

    /**
     * \brief Обработка события завершения обработки ответа
     * \details Данный сигнал срабатывает когда ответ считается обработанным (единожды за ответ)
     */
    void onReplyFinished();

    /**
     * \brief Клик по конопке "пуск"
     */
    void on_btnParseStart_clicked();

    /**
     * \brief Клик по конопке "стоп"
     */
    void on_btnParsePause_clicked();

    /**
     * \brief Клик по конопке "пауза"
     */
    void on_btnParseStop_clicked();

    /**
     * \brief Клик по кнопке "фильтрация"
     */
    void on_btnFilter_clicked();

private:
    /// Указатель на UI объект
    Ui::MainWindow* ui_;

    /// Объект для работы с HTTP запросами
    QNetworkAccessManager* httpManager_;
    /// Текущий URL для запросов
    QUrl httpUrl_ = QUrl("");
    /// Указатель на объект HTTP ответа
    QNetworkReply* httpReply_ = nullptr;

    /// Содержимое последнего ответа (HTML)
    QString downloadedHtml_ = "";
    /// Общее кол-вл друзей (значение обновляется после первого парсинга)
    quint32 friendsTotal_ = 0;
    /// Сет для избежания добавления уже добавленных
    QSet<QString> friendsAdded_;

    /// Статус
    Status status_ = Status::eStopped;
    /// Первая итерация
    bool firstIteration_ = true;


    /**
     * \brief Обновить комбо-бокс с выбором агентов
     */
    void initAgentsCombo();

    /**
     * \brief Инициировать HTTP запрос, зарегистрировать обработчики событий для чтения ответа
     */
    void makeHttpRequest();

    /**
     * \brief Обновить индикатор статуса
     */
    void updateStatusLabel();

    /**
     * \brief Получить кол-во друзей из HTML
     * \return Удалось ли получить
     */
    bool parseFriendsTotalCount();

    /**
     * \brief Получить следующий URL из HTML
     * \return Удалось ли получить
     */
    bool parseNextUrl();

    /**
     * \brief Получить данные о друзьях из HTML
     * \return Удалось ли получить
     */
    bool parseFriends();

    /**
     * \brief Фильтрация таблицы
     * \param conditionString Критерий
     */
    void applyFiltration(const QString& conditionString);
};
