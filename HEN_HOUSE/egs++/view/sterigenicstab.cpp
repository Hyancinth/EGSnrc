#include "sterigenicstab.h"

sterigenicsTab::sterigenicsTab(QWidget *parent, const char *name) :
    QWidget(parent),
    ui(new Ui::sterigenicsTab)
{
    ui->setupUi(this);
    pushCounter = 0;
    
    const char* q[4] = {"a", "b", "c", "d"};

    for (int i = 0; i < 4; i++){
        ui -> comboBox -> insertItem(i, q[i]);
    }

    // signaler, signal, slotter, slot
    connect(ui->pushButton, &QPushButton::clicked, this, &sterigenicsTab::pushButtonClicked); // more modern notation 
    connect(this, &sterigenicsTab::counterReached, this, &sterigenicsTab::changeButtonColour); //more modern notation

}

sterigenicsTab::~sterigenicsTab()
{
    delete ui;
}

void sterigenicsTab::pushButtonClicked(bool checked)
{
    if (checked){
        ui->pushButton->setText("BUH");
    }
    else{
        ui->pushButton->setText("Hello World");  
    }

    ui->testL->setText(ui->comboBox->currentText());
    pushCounter++;
    if(pushCounter == 5){
        emit counterReached();
    }
}

// slot that changes the button colour after the signal (counterReached) is emitted
void sterigenicsTab::changeButtonColour(){
    ui->pushButton->setText(ui->comboBox->currentText());  
    ui->pushButton->setStyleSheet("background-color: red;");
}