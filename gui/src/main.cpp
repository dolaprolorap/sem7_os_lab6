#include <QApplication>
#include <QVector>
#include <QTimer>
#include <cstdlib>
#include <QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtCore>
#include <QDateTimeEdit>
#include <iostream>
#include <QVBoxLayout>
#include <QLabel>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_text.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget window;
    QVBoxLayout *layout = new QVBoxLayout(&window);

    QHBoxLayout *startContainer = new QHBoxLayout;
    QHBoxLayout *endContainer = new QHBoxLayout;

    QLabel *startLabel = new QLabel("&Начало");
    QLabel *endLabel = new QLabel("&Конец");

    QDateTimeEdit *startDate = new QDateTimeEdit(QDate::currentDate().addDays(-1));
    startDate->setMinimumDateTime(QDateTime::currentDateTime().addDays(-10));
    startDate->setMaximumDateTime(QDateTime::currentDateTime().addDays(10));
    startDate->setDisplayFormat("dd-MM-yyyy hh-mm-ss");

    QDateTimeEdit *endDate = new QDateTimeEdit(QDate::currentDate().addDays(1));
    endDate->setMinimumDateTime(QDateTime::currentDateTime().addDays(-10));
    endDate->setMaximumDateTime(QDateTime::currentDateTime().addDays(10));
    endDate->setDisplayFormat("dd-MM-yyyy hh-mm-ss");

    startLabel->setBuddy(startDate);
    endLabel->setBuddy(endDate);

    startContainer->addWidget(startLabel);
    startContainer->addWidget(startDate);

    endContainer->addWidget(endLabel);
    endContainer->addWidget(endDate);

    QwtPlot plot;
    plot.setTitle("Дневная температура");
    plot.setAxisTitle(QwtPlot::xBottom, "Отсчёты");
    plot.setAxisTitle(QwtPlot::yLeft, "Температура (t)");
    plot.resize(800, 600);

    QwtPlotCurve curve;
    curve.attach(&plot);

    layout->addWidget(&plot);

    layout->addLayout(startContainer);
    layout->addLayout(endContainer);
    
    QNetworkAccessManager manager;
    QString url;
    QEventLoop event;

    auto updateStartFinish = [&]() 
    {
        qint64 start = startDate->dateTime().toSecsSinceEpoch();
        qint64 finish = endDate->dateTime().toSecsSinceEpoch();

        url = QString("http://127.0.0.1:8080/data?finish=%1&start=%2").arg(finish).arg(start);

        printf("%d\n", start);
        printf("%d\n", finish);
    };

    QObject::connect(
        startDate,
        &QDateTimeEdit::dateTimeChanged,
        [&](const QDateTime&) { updateStartFinish(); }
    );

    QObject::connect(
        endDate,
        &QDateTimeEdit::dateTimeChanged,
        [&](const QDateTime&) { updateStartFinish(); }
    );

    updateStartFinish();

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]() {
        QNetworkReply *response = manager.get(QNetworkRequest(QUrl(url)));
        QObject::connect(response, &QNetworkReply::finished, &event, &QEventLoop::quit);
        event.exec();

        if (response->error() != QNetworkReply::NoError)
        {
            qWarning() << "Ошибка запроса: " << response->errorString();

            return;
        }

        QString content = response->readAll();
        QJsonDocument json = QJsonDocument::fromJson(content.toUtf8());

        QVector<double> x_new;
        QVector<double> y_new;

        for (int i = 0; i < json.array().size(); i++)
        {
            x_new.push_back(i);
            y_new.push_back(json.array()[i].toDouble());
        }

        curve.setSamples(x_new.data(), y_new.data(), y_new.size());
        plot.replot();
    });
    timer.start(1000);

    window.show();

    return app.exec();
}
