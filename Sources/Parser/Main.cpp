#include <QApplication>
#include "MainWindow.h"

/**
 * \brief Точка входа
 * \param argc Кол-во аргументов
 * \param argv Аргументы
 * \return Выполнить
 */
int main(int argc, char* argv[])
{
    // Инициализация приложения
    QApplication a(argc, argv);

    // Создать и показать основной окно
    MainWindow mainWindow;
    mainWindow.show();

    // Звпуск
    return QApplication::exec();
}