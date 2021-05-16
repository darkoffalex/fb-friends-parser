#include "MainWindow.h"
#include "./ui_MainWindow.h"

/**
 * \brief Конструктор основного окна
 * \param parent Указатель на родительский виджет
 */
MainWindow::MainWindow(QWidget *parent) :
        QWidget(parent),
        ui_(new Ui::MainWindow()),
        replyInitial_(nullptr),
        replyRegular_(nullptr)
{
    // Инициализация UI
    ui_->setupUi(this);

    // Инициализация объекта для работы с сетью
    httpManager_ = new QNetworkAccessManager(this);
    //connect(httpAccessManager_, &QNetworkAccessManager::finished, this, &MainWindow::httpInitialRequestFinished);
}

/**
 * \brief Деструктор
 */
MainWindow::~MainWindow()
{
    // Уничтожение UI
    delete ui_;

    // Уничтожение объекта для работы с сетью
    delete httpManager_;
}

/** S L O T S **/

/**
 * \brief Обработка события клика по кнопке подключения
 */
void MainWindow::on_btnConnect_clicked()
{
    // URL запроса
    QUrl url("https://mbasic.facebook.com/adezhurov/friends");

    // Создать запрос
    QNetworkRequest request(url);
    request.setRawHeader("Accept","text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
    request.setRawHeader("Accept-Encoding","utf-8");
    request.setRawHeader("Accept-Language","ru-RU,ru;q=0.8,en-US;q=0.5,en;q=0.3");
    request.setRawHeader("Cache-Control","max-age=0");
    request.setRawHeader("Connection","keep-alive");
    request.setRawHeader("Cookie","datr=KnOeYNz5xBpkpz6ZJ2lKmINO; sb=h3OeYEsORctrHjS8bIJq7MYl; c_user=100012840103209; xs=44%3AODRc3FvypQrxIg%3A2%3A1620996999%3A-1%3A14350%3A%3AAcUWw2he7Va1LtVR8nON-BOieZznMOw_K6RIjz_AJQ; fr=1j1ylPbtEWadU1vb3.AWXvwLm2U6leXV3m2eRgOjRYfzA.BgoECX.IO.AAA.0.0.BgoECX.AWUNN7CcBe8; wd=1920x929; spin=r.1003799531_b.trunk_t.1621115027_s.1_v.2_");
    request.setRawHeader("Host","mbasic.facebook.com");
    request.setRawHeader("TE","Trailers");
    request.setRawHeader("Upgrade-Insecure-Requests","1");
    request.setRawHeader("User-Agent","Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:88.0) Gecko/20100101 Firefox/88.0");

    // Выполнить GET запрос
    this->replyInitial_ = this->httpManager_->get(request);

    // События ответа
    connect(replyInitial_, &QIODevice::readyRead, this, &MainWindow::onInitialReplyReadyRead);
    connect(replyInitial_, &QNetworkReply::errorOccurred, this, &MainWindow::onInitialReplyError);
}

void MainWindow::onInitialReplyReadyRead()
{
    this->ui_->textBoxCookie->setPlainText(replyInitial_->readAll());

//    QMessageBox msg;
//    msg.setTextFormat(Qt::TextFormat::PlainText);
//    msg.setText(replyInitial_->readAll());
//    msg.show();
//    msg.exec();
}

void MainWindow::onInitialReplyError(QNetworkReply::NetworkError code)
{
    qDebug() << code;
}
