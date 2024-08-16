#ifndef STERIGENICSTAB_H
#define STERIGENICSTAB_H

#include <QWidget>
#include "ui_sterigenicstab.h"

namespace Ui {
class sterigenicsTab;
}

class sterigenicsTab : public QWidget
{
    Q_OBJECT

public:
    explicit sterigenicsTab(QWidget *parent = 0, const char *name = 0);
    ~sterigenicsTab(); // deconstructor

public slots:
    void pushButtonClicked(bool checked);
    void changeButtonColour();
    void inputButtonClicked();
    void runSimButtonClicked();
    void openFileButtonClicked();

    void onSimulationFinished(const QString& simOutput);
    void onErrorOccurred(const QString& err);

signals:
    void counterReached();

private:
    Ui::sterigenicsTab *ui;
    int pushCounter;
};

#endif // STERIGENICSTAB_H
