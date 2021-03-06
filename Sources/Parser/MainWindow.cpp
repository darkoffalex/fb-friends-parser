#include "MainWindow.h"
#include "./ui_MainWindow.h"

/**
 * \brief Конструктор основного окна
 * \param parent Указатель на родительский виджет
 */
MainWindow::MainWindow(QWidget *parent) :
        QWidget(parent),
        ui_(new Ui::MainWindow()),
        httpManager_(nullptr)
{
    // Инициализация UI
    ui_->setupUi(this);

    // Инициализировать HTTP
    httpManager_ = new QNetworkAccessManager(this);

    // Инициализирвоать комбо-бокс с аггентами
    this->initAgentsCombo();
}

/**
 * \brief Деструктор
 */
MainWindow::~MainWindow()
{
    // Уничтожение UI
    delete ui_;

    // Уничтожение объекта для работы HTTP
    delete httpManager_;
}

/**
 * \brief Обновить комбо-бокс с выбором агентов
 */
void MainWindow::initAgentsCombo()
{
    // Список агентов и соответствующие заголовки
    QMap<QString,UserAgentType> agents = {
            {"Mozilla Firefox", UserAgentType::eMozillaFirefox},
            {"Google Chrome",   UserAgentType::eGoogleChrome}
    };

    // Пройтись по списку и обновить комбо-бокс
    const auto& m = agents.toStdMap();
    for(const auto& pair : m)
    {
        this->ui_->comboBrowser->addItem(pair.first,static_cast<quint32>(pair.second));
    }

    // Выбрать последний вариант
    this->ui_->comboBrowser->setCurrentIndex(ui_->comboBrowser->count()-1);
}

/**
 * \brief Инициировать HTTP запрос, зарегистрировать обработчики событий для чтения ответа
 */
void MainWindow::makeHttpRequest()
{
    // Куки
    QByteArray cookie = QByteArray::fromStdString(ui_->editCookie->toPlainText().toStdString());

    // Заголовок агента (для эмуляции бразуера)
    UserAgentType headers = static_cast<UserAgentType>(ui_->comboBrowser->currentData().toInt());

    // Создать запрос
    QNetworkRequest request(httpUrl_);

    // Добавление заголовков в зависимости от браузера
    switch (headers)
    {
        case UserAgentType::eMozillaFirefox:
        default:
            request.setRawHeader("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
            request.setRawHeader("Accept-Encoding","utf-8");
            request.setRawHeader("Accept-Language","ru-RU,ru;q=0.8,en-US;q=0.5,en;q=0.3");
            request.setRawHeader("Cache-Control","max-age=0");
            request.setRawHeader("Connection","keep-alive");
            request.setRawHeader("Cookie",cookie);
            request.setRawHeader("Host","mbasic.facebook.com");
            request.setRawHeader("TE","Trailers");
            request.setRawHeader("Upgrade-Insecure-Requests","1");
            request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:88.0) Gecko/20100101 Firefox/88.0");
            break;

        case UserAgentType::eGoogleChrome:
            request.setRawHeader("accept","text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9");
            request.setRawHeader("accept-encoding","utf-8");
            request.setRawHeader("accept-language","ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7");
            request.setRawHeader("cache-control","max-age=0");
            request.setRawHeader("cookie",cookie);
            request.setRawHeader("sec-ch-ua",R"(" Not A;Brand";v="99", "Chromium";v="90", "Google Chrome";v="90")");
            request.setRawHeader("sec-ch-ua-mobile","?0");
            request.setRawHeader("sec-fetch-dest","document");
            request.setRawHeader("sec-fetch-mode","navigate");
            request.setRawHeader("sec-fetch-site","none");
            request.setRawHeader("sec-fetch-user","?1");
            request.setRawHeader("upgrade-insecure-requests","1");
            request.setRawHeader("user-agent","Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/90.0.4430.212 Safari/537.36");
            break;
    }

    // Очистить загруженные данные
    this->downloadedHtml_.clear();

    // Выполнить GET запрос
    this->httpReply_ = this->httpManager_->get(request);

    // Связать ответ с обработчками событий
    connect(httpReply_, &QIODevice::readyRead, this, &MainWindow::onReplyReadyRead);
    connect(httpReply_, &QNetworkReply::errorOccurred, this, &MainWindow::onReplyError);
    connect(httpReply_, &QNetworkReply::finished, this, &MainWindow::onReplyFinished);
}

/**
 * \brief Обновить индикатор статуса
 */
void MainWindow::updateStatusLabel()
{
    // Кол-во друзей
    QString friendsCounterLabel =
            "(" + QString::number(friendsAdded_.size()) +
            "/" + QString::number(friendsTotal_) + " друзей)";

    // Названия состояний
    QMap<Status,QString> statusNames = {};
    statusNames[Status::eStopped] = "Остановлено";
    statusNames[Status::ePaused] = "Приостановлено";
    statusNames[Status::eParsing] = "Сбор данных";

    // Установить текст
    this->ui_->labelStatusName->setText(statusNames[this->status_] + " " + friendsCounterLabel);
}

/**
 * \brief Получить кол-во друзей из HTML
 * \return Удалось ли получить
 */
bool MainWindow::parseFriendsTotalCount()
{
    // Регулярные выражения для извлечения кол-ва
    QRegularExpression rx0(this->getRegExPattern(ParsingPatternType::eFriendsTotalCount));
    QRegularExpression rx1("\\((.+?)\\)");

    // Если было совпадение по первому паттерну
    auto match = rx0.match(this->downloadedHtml_);
    if(match.hasMatch() && match.capturedTexts().size() > 1)
    {
        // Если было совпадение по второму маттерну
        match = rx1.match(match.captured(1));
        if(match.hasMatch() && match.capturedTexts().size() > 1)
        {
            // Удалось ли конвертировать строку в число
            bool converted = false;
            // Попытка конвертировать строку в число
            auto number = match.captured(1).simplified().replace(" ","").toInt(&converted);
            // Установить новое значение
            this->friendsTotal_ = converted ? number : 0;
            // Статус операции
            return converted;
        }
    }

    // Парсинг не удался
    return false;
}

/**
 * \brief Получить следующий URL из HTML
 * \return Удалось ли получить
 */
bool MainWindow::parseNextUrl()
{
    // Регулярное выражение для получения следующего URL
    QRegularExpression rx(this->getRegExPattern(ParsingPatternType::eNextUrl));

    // Если было совпадение по паттерну
    auto match = rx.match(this->downloadedHtml_);
    if(match.hasMatch() && match.capturedTexts().size() > 1)
    {
        auto part = match.captured(1);
        this->httpUrl_ = QUrl("https://mbasic.facebook.com" + part);
        return true;
    }

    return false;
}

/**
 * \brief Получить данные о друзьях из HTML
 * \return Удалось ли получить
 */
bool MainWindow::parseFriends()
{
    // Регулярное выражение для получения данных о друзях
    QRegularExpression rx0(this->getRegExPattern(ParsingPatternType::eFriendEntry));
    // Регулярное выражения для получения извлечения информации о работе
    QRegularExpression rx1(this->getRegExPattern(ParsingPatternType::eFriendEntryInfo));

    // Успешно ли получены данные
    bool parsed = false;

    // Выполнить глобальный поиск
    auto it = rx0.globalMatch(this->downloadedHtml_);

    // Если есть следущий итератор
    while (it.hasNext())
    {
        // Получить совпадение
        auto match = it.next();

        // Если совпадение действительно было
        if(match.hasMatch())
        {
            // Парсинг успешен
            parsed = true;

            // Получить необходимые строки
            auto link = match.captured(1);
            auto name = match.captured(2);
            auto info = match.captured(3);

            // Если ссылка на профиль еще не фигурировала
            if(!friendsAdded_.contains(link))
            {
                // Внести ссылку на профиль в set
                friendsAdded_.insert(link);

                // Привести ссылку в удобоваримый вид
                link.replace("?fref=fr_tab","");
                link.replace("&amp;fref=fr_tab","");
                link = "https://www.facebook.com" + link;

                // Создать в таблице запись
                auto table = this->ui_->tableWidget;
                table->insertRow(table->rowCount());
                table->setItem(table->rowCount()-1,0,new QTableWidgetItem(name));
                table->setItem(table->rowCount()-1,1,new QTableWidgetItem(link));

                // Если есть информация о работе
                auto workMatch = rx1.match(info);
                if(workMatch.hasMatch() && workMatch.capturedTexts().size() > 1)
                {
                    table->setItem(table->rowCount()-1,2,new QTableWidgetItem(workMatch.captured(1)));
                }
            }
        }
    }

    return parsed;
}

/**
 * \brief Фильтрация таблицы
 * \param conditionString Критерий
 */
void MainWindow::applyFiltration(const QString &conditionString)
{
    // Показать все записи
    for(int i = 0; i < ui_->tableWidget->rowCount(); i++)
        ui_->tableWidget->showRow(i);

    // Если критерий указан
    if(!conditionString.isEmpty())
    {
        // Скрыть все записи
        for(int i = 0; i < ui_->tableWidget->rowCount(); i++)
            ui_->tableWidget->hideRow(i);

        // Найти записи удовлетворяющие критерии
        auto items = ui_->tableWidget->findItems(conditionString,Qt::MatchContains);

        // Показать найденные записи
        for(auto item : items)
            ui_->tableWidget->showRow(item->row());
    }

}

/**
 * \brief Получить паттерн для регулярного выражения
 * \param type Тип паттерна
 * \details Верстка меняется в зависимость от браузера и от того, главная ли это страница. Паттерны должны это учитывать
 * \return Строка с паттерном
 */
QString MainWindow::getRegExPattern(MainWindow::ParsingPatternType type)
{
    // Информация о выбранном браузере
    auto agent = static_cast<UserAgentType>(this->ui_->comboBrowser->currentData().toInt());

    // Паттерны (на каждый бразуер свой)
    QMap<UserAgentType,QString> patterns;

    // В зависимости от типа паттерна
    switch (type)
    {
        // Паттерн для получения кол-ва друзей
        case ParsingPatternType::eFriendsTotalCount:
        default:
        {
            patterns[UserAgentType::eMozillaFirefox] = this->firstIteration_ ? R"(<h3 class="ca k">(.+?)<\/h3>)" : R"(<h3 class="bk k">(.+?)<\/h3>)";
            patterns[UserAgentType::eGoogleChrome] = this->firstIteration_ ? R"(<h3 class="bz j">(.+?)<\/h3>)" : R"(<h3 class="bj j">(.+?)<\/h3>)";
            break;
        }

        // Паттерн для получения ссылки на след. страницу (не меняется)
        case ParsingPatternType::eNextUrl:
        {
            patterns[UserAgentType::eMozillaFirefox] = "id=\"m_more_friends\"><a href=\"(.+?)\"><span>";
            patterns[UserAgentType::eGoogleChrome] = "id=\"m_more_friends\"><a href=\"(.+?)\"><span>";
            break;
        }

        // Паттерн для извлечения данных по друзьям
        case ParsingPatternType::eFriendEntry:
        {
            patterns[UserAgentType::eMozillaFirefox] = this->firstIteration_ ? "<a class=\"ce\" href=\"(.+?)\">(.+?)<\\/a><div class=\"cf cg\">(.+?)<\\/div>" : "<a class=\"bp\" href=\"(.+?)\">(.+?)<\\/a><div class=\"bq br\">(.+?)<\\/div>";
            patterns[UserAgentType::eGoogleChrome] = this->firstIteration_ ? "<a class=\"cd\" href=\"(.+?)\">(.+?)<\\/a><div class=\"ce cf\">(.+?)<\\/div>" : "<a class=\"bo\" href=\"(.+?)\">(.+?)<\\/a><div class=\"bp bq\">(.+?)<\\/div>";
            break;
        }

        // Паттерн для изалчения подробной информации
        case ParsingPatternType::eFriendEntryInfo:
        {
            patterns[UserAgentType::eMozillaFirefox] = this->firstIteration_ ? R"(<span class="by">(.+?)<\/span>)" : R"(<span class="bs">(.+?)<\/span>)";
            patterns[UserAgentType::eGoogleChrome] = this->firstIteration_ ? R"(<span class="bx">(.+?)<\/span>)" : R"(<span class="br">(.+?)<\/span>)";
            break;
        }
    }

    return patterns[agent];
}

/** S L O T S **/

/**
 * \brief Обработчка события готовности чтения ответа
 * \details Данный сигнал может срабатывать НЕСКОЛЬКО раз при обработке ОДНОГО ответа (данные загружаются порциями)
 */
void MainWindow::onReplyReadyRead()
{
    // Пополнить буфер данными
    this->downloadedHtml_ += this->httpReply_->readAll();
}

/**
 * \brief Обработка события ошибки ответа
 * \param code Код ошибки
 */
void MainWindow::onReplyError(QNetworkReply::NetworkError code)
{
    // Сообщение об ошибке
    QMessageBox msgBox{};
    msgBox.setText("Ошибка при чтении ответа (код - " + QString::number(code) + ")");
    msgBox.setWindowTitle("Ошибка соединения");
    msgBox.setIcon(QMessageBox::Icon::Warning);
    msgBox.show();
    msgBox.exec();

    // Сменить статус
    this->status_ = Status::eStopped;
    this->updateStatusLabel();
}

/**
 * \brief Обработка события завершения обработки ответа
 * \details Данный сигнал срабатывает когда ответ считается обработанным (единожды за ответ)
 */
void MainWindow::onReplyFinished()
{
    // Парсинг основных данных
    bool countParsed = this->parseFriendsTotalCount();
    bool urlParsed = this->parseNextUrl();
    bool friendsParsed = this->parseFriends();

    // Продолжать парсинг
    bool continueParsing = true;

    // Если статус был изменен на "остановлено" или же не были получены какие-то данные
    if(status_ == Status::eStopped || !(countParsed && urlParsed && friendsParsed))
    {
        // Если остановка произошла не по причине ручной остановки, а по причине невозможности получения данных
        if(status_ != Status::eStopped)
        {
            // Сообщение об ошибке
            QMessageBox msgBox{};
            msgBox.setText("Не удалось получить необходимые данные из загруженного HTML кода. Процесс будет остановлен.");
            msgBox.setWindowTitle("Остановка процесса");
            msgBox.setIcon(QMessageBox::Icon::Information);
            msgBox.show();
            msgBox.exec();
        }

        // Сменить статус
        this->status_ = Status::eStopped;

        // Интерфейс
        this->ui_->gbAuth->setEnabled(true);
        this->ui_->btnParseStart->setEnabled(true);
        this->ui_->btnParseStop->setEnabled(false);
        this->ui_->btnParsePause->setEnabled(false);
        this->ui_->btnFilter->setEnabled(true);

        // Парсинг остановлен
        continueParsing = false;
    }
    // Если статус был изменен на "приостановлено"
    else if(status_ == Status::ePaused)
    {
        // Интерфейс
        this->ui_->gbAuth->setEnabled(false);
        this->ui_->btnParseStart->setEnabled(true);
        this->ui_->btnParseStop->setEnabled(false);
        this->ui_->btnParsePause->setEnabled(false);
        this->ui_->btnFilter->setEnabled(true);

        // Парсинг остановлен
        continueParsing = false;
    }

    // Обновить информацию о статусе
    this->updateStatusLabel();
    // Это более не первая итерация
    this->firstIteration_ = false;
    // Очистить HTML
    this->downloadedHtml_.clear();
    // Закрыть соединение
    this->httpReply_->close();

    // Если парсинг продолжается
    if(continueParsing)
    {
        // Инициировать новый запрос (следующая страница)
        this->makeHttpRequest();
    }
}

/**
 * \brief Клик по конопке "пуск"
 */
void MainWindow::on_btnParseStart_clicked()
{
    // Убрать фильтрацию с таблицы
    this->applyFiltration("");

    // Если отсутствует имя пользователя
    if(this->ui_->editUsername->text().isEmpty() || this->ui_->editCookie->toPlainText().isEmpty())
    {
        // Сообщение об ошибке
        QMessageBox msgBox{};
        msgBox.setText("Не введены необходимые данные. Невозможно начать процесс.");
        msgBox.setWindowTitle("Ошибка");
        msgBox.setIcon(QMessageBox::Icon::Warning);
        msgBox.show();
        msgBox.exec();
        return;
    }

    // Интерфейс
    this->ui_->gbAuth->setEnabled(false);
    this->ui_->btnParseStart->setEnabled(false);
    this->ui_->btnParseStop->setEnabled(true);
    this->ui_->btnParsePause->setEnabled(true);
    this->ui_->btnFilter->setEnabled(false);

    // Если последнее состояние - "остановлено"
    if(this->status_ == Status::eStopped)
    {
        // Значит это первая итерация парсинга
        this->firstIteration_ = true;
        // Очистить таблицу друзей
        this->ui_->tableWidget->setRowCount(0);
        // Очистить set друзей
        this->friendsAdded_.clear();
        // Обнулить кол-во
        this->friendsTotal_ = 0;
        // Сформировать начальный URL
        this->httpUrl_ = QUrl("https://mbasic.facebook.com/" + this->ui_->editUsername->text() + "/friends");
    }

    // Сменить статус
    this->status_ = Status::eParsing;
    this->updateStatusLabel();

    // Выполнить первый запрос
    this->makeHttpRequest();
}

/**
 * \brief Клик по конопке "стоп"
 */
void MainWindow::on_btnParsePause_clicked()
{
    this->ui_->btnParsePause->setEnabled(false);
    this->status_ = Status::ePaused;
}

/**
 * \brief Клик по конопке "пауза"
 */
void MainWindow::on_btnParseStop_clicked()
{
    this->ui_->btnParseStop->setEnabled(false);
    this->status_ = Status::eStopped;
}

/**
 * \brief Клик по кнопке "фильтрация"
 */
void MainWindow::on_btnFilter_clicked()
{
    this->applyFiltration(this->ui_->editFilter->text());
}

