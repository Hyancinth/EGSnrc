/**
 * EGS View tab extension for Sterigenics
 * Creates and runs EGSnrc simulations based on GUI inputs
 * Worker function to run egs sim process on a different thread. 
 * 
 * TO DO:
 *      Full functionality has not been implemented, specifically with placing the mesh within the product
 *      As well all of the required parameters needed to be inputted is unknown at the moment 
 * 
 * Author: Ethan Tran , 2024
*/

#include "sterigenicstab.h"
#include "sterigenicsWorker.h"
#include <QProcess> 
#include <QString>
#include <QDebug>
#include <string>
#include <cstdio> 
#include <cstdlib>
#include <QStringList>
#include <QTextStream>
#include <QFile>
#include <QDir>
#include <QList>
#include <QObject>
#include <vector>
#include <limits> 
#include <QFileDialog>
#include <QThread>


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
    //test
    // connect(ui->pushButton, &QPushButton::clicked, this, &sterigenicsTab::pushButtonClicked); // more modern notation 
    connect(this, &sterigenicsTab::counterReached, this, &sterigenicsTab::changeButtonColour); //more modern notation
    connect(ui->inputSubmit, &QPushButton::clicked, this, &sterigenicsTab::inputButtonClicked);
    connect(ui->runSim, &QPushButton::clicked, this, &sterigenicsTab::runSimButtonClicked);
    connect(ui -> openFileButton, &QPushButton::clicked, this, &sterigenicsTab::openFileButtonClicked);
}

sterigenicsTab::~sterigenicsTab()
{
    delete ui;
}
 
// TEST 
void sterigenicsTab::pushButtonClicked(bool checked)
{
    // // process for running python script
    // QProcess process;
    // QString scriptFile =  "/home/trane/Desktop/EGSnrc/HEN_HOUSE/egs++/view/testPython.py";
    // //To be more general, use this: QCoreApplication::applicationDirPath() + "from the applicationDirPath() to the python file"

    // // create and run python command: two options:
    // // using QString:
    // // QString pythonCommand = "python " + scriptFile + 
    // //                 " -f " + parameter1 +
    // //                 " -t parameter2" +
    // //                 " -v parameter3" +
    // //                 " -e " + parameter4;
    // // process.start (pythonCommand);


    // // using QStringList:
    // // QStringList pythonCommandArguments = QStringList() << scriptFile
    // //      << "-f " << parameter1 << "-t" <<  parameter2 << "-v"
    // //      <<  parameter3 << "-e" << parameter4;
    // //process.start ("python", pythonCommandArguments);

    // QString pythonCommand = "python " + scriptFile; 
    // process.start(pythonCommand);
    // if (!process.waitForFinished()) {
    //     QString pError = process.errorString();
    //     qDebug() << "Error: " << process.errorString();
    //     // egsInformation("Error: %s", process.errorString().toStdString());
    //     printf("Process Error: %s", pError.toStdString().c_str());
    // }


    // // read output of python file
    // QString output = process.readAllStandardOutput();
    // // egsInformation("%s", output.toStdString());
    // printf("%s", output.toStdString().c_str()); //convert from QString to std::String to c string for printf
    // qDebug() << "Output:" << output;

    // QString outError = process.readAllStandardError();
    // if (!outError.isEmpty()) {
    //     printf("All Standard Error: %s", outError.toStdString().c_str());
    // }

    // writing to a file
    QString filePath = QDir::homePath() + "/Desktop/testOut.py";
    
    QFile outFile(filePath);
    printf("Before\n");
    fflush(stdout);
    if(outFile.open(QIODevice::WriteOnly | QIODevice::Append)){
        QTextStream stream(&outFile);
        stream << "Hello World\nbuh" << endl;
        stream.flush();
        printf("Written\n");
        fflush(stdout);
    }
    printf("After\n");
    fflush(stdout);

    if (checked){
        ui->pushButton->setText("BUH");
    }
    else{
        ui->pushButton->setText("Hello World");  
    }

    ui->warningLabel->setText(ui->comboBox->currentText());
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

void sterigenicsTab::inputButtonClicked(){
    /**
     * Creates python file based on user inputs and runs egs-rollout
     * 
    */
    // get all labels and input texts
    int emptyFlag = 0; 
    QList<QLineEdit *> inputList = ui -> verticalLayoutWidget->findChildren<QLineEdit *>();
    std::vector<QString> allInputs;
    printf("%s \n", (ui-> meshFileLabel->text()).toStdString().c_str());
    for (QLineEdit *input : inputList){
        if(input->text().isEmpty() || ui-> meshFileLabel->text() == "file name"){
            ui -> warningLabel -> setText("Please do not leave inputs blank | Please Load a Mesh");
            emptyFlag = 1;
            allInputs.clear();
            inputList.clear();
            break;
        }
        else{
            emptyFlag = 0;
            ui -> warningLabel -> setText("");
            allInputs.push_back(input ->text());
        }
    }


    // if there are no empty inputs and a mesh file is loaded 
    if(emptyFlag == 0){
        QList<QLabel *> labelList = ui -> verticalLayoutWidget->findChildren<QLabel *>(); // get all labels in the widget called: verticalLayoutWidget
        std::vector<QString> allLabels; 

        for (QLabel *label : labelList){
            allLabels.push_back(label -> text());
        }
        allLabels.erase(allLabels.begin()); //remove this if the warning label is removed 

        
        QString meshName = allLabels[0]; // get the meshName from labels
        //move the meshName from labels into inputs and set the corresponding label. 
        //mesh stuff is put at the front of each vector. 
        allLabels[0] = "meshName";
        allInputs.insert(allInputs.begin(), meshName);

        // printf("labels: %s | %s \n", allLabels[0].toStdString().c_str(), allLabels[1].toStdString().c_str());
        // printf("inputs: %s | %s \n", allInputs[0].toStdString().c_str(), allInputs[1].toStdString().c_str());
        // fflush(stdout);

        // creating the .py file
        std::string paramFile;
        std::string setFunc;
        std::string computeFunc;

        paramFile += "#!/usr/bin/python\nimport os\n";

        setFunc += "def set():\n\tparam = dict()\n";
        setFunc += "\tparam['template'] = 'guiTest.template'\n";
        setFunc += "\tparam['label'] = '_%(n_case)s_%(windowMaterial)s'\n";
        setFunc += "\tparam['user_path'] = os.path.join(os.getenv('EGS_HOME'), 'mevegs')\n";

        for (std::size_t i = 0; i < allInputs.size(); i++){
            setFunc = setFunc + "\tparam['" + allLabels[i].toStdString() + "'] = '" + allInputs[i].toStdString() + "'\n"; 
        } 

        setFunc += "\treturn param\n";

        computeFunc += "def compute(param): \n";
        computeFunc += "\treturn param";

        paramFile = paramFile + setFunc + computeFunc;

        printf("%s \n", paramFile.c_str());
        fflush(stdout);

        QString paramFileName = "testParam";

        // QString filePath = QDir::homePath() + "/Desktop/EGSnrc/egs_home/mevegs/" + paramFileName +".py";
        QString filePath;
        filePath = filePath + getenv("EGS_HOME") + "mevegs";

        // write to output file
        QFile outFile(filePath);
        printf("Before\n");
        fflush(stdout);
        if(outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)){
            QTextStream stream(&outFile);
            QString qParamFile = QString::fromStdString(paramFile);  
            stream << qParamFile << endl;
            stream.flush();
            printf("Written\n");
            outFile.close();
            fflush(stdout);
        }
        printf("After\n");
        fflush(stdout);

        allInputs.clear();
        inputList.clear();
        qDeleteAll(inputList.begin(), inputList.end());

        allLabels.clear();
        labelList.clear();
        qDeleteAll(labelList.begin(), labelList.end());

        
        // run chmod command (egs-rollout into an executable)
        QProcess chmodProcess;

        // QString chmodCommand = "chmod a+x " + QDir::homePath() + "/Desktop/EGSnrc/egs_home/mevegs/egs-rollout";
        QString chmodCommand; 
        chmodCommand = chmodCommand + "chmod a+x " + getenv("EGS_HOME") + "mevegs/egs-rollout";

        chmodProcess.start(chmodCommand);

        if (!chmodProcess.waitForFinished()) {
            QString chmodError = chmodProcess.errorString();
            printf("Chmod Process Error: %s\n", chmodError.toStdString().c_str());
        }


        // run egs-rollout command 
        printf("start egs-rollout\n");

        // QString egsRolloutPath = QDir::homePath() + "/Desktop/EGSnrc/egs_home/mevegs";
        QString egsRolloutPath;
        egsRolloutPath = egsRolloutPath + getenv("EGS_HOME") + "mevegs";

        QString egsRolloutCommand = egsRolloutPath + "/egs-rollout " + paramFileName;
        
        printf("%s\n", egsRolloutCommand.toStdString().c_str());

        QProcess egsRolloutProcess; 
        egsRolloutProcess.start(egsRolloutCommand);

        if(!egsRolloutProcess.waitForFinished()){
            QString egsRolloutError = egsRolloutProcess.errorString();
            printf("Process Error: %s\n", egsRolloutError.toStdString().c_str());
        }
        QString egsRolloutOutput = egsRolloutProcess.readAllStandardOutput();
        printf("%s \n", egsRolloutOutput.toStdString().c_str());

        printf("done egs-rollout\n");

    }

}


void sterigenicsTab::runSimButtonClicked(){
    /**
     * Runs the egsinp file created by egs-rollout (from the above function)
    */
    int emptyFlag = 0; 
    int timeoutLimit = std::numeric_limits<int>::max();

    // check that the simulation params are filled 
    QList<QLineEdit *> inputList = ui -> verticalLayoutWidget->findChildren<QLineEdit *>();
    std::vector<QString> allInputs;
    for (QLineEdit *input : inputList){
        if(input->text().isEmpty()){
            ui -> warningLabel -> setText("Please enter simulation params");
            emptyFlag = 1;
            allInputs.clear();
            inputList.clear();
            break;
        }
        else{
            emptyFlag = 0;
            ui -> warningLabel -> setText("");
            allInputs.push_back(input ->text());
        }
    }

    if(emptyFlag == 0){

        QString templatePath;
        templatePath = templatePath + getenv("EGS_HOME") + "mevegs";
        printf("%s \n", templatePath.toStdString().c_str());

        QStringList nameFilter;
        nameFilter << "*.template";

        QDir templateDir(templatePath);
        
        QStringList templateFiles = templateDir.entryList(nameFilter, QDir::Files);
        QStringList parts = templateFiles[0].split(".");
        QString templateName = parts[0];
        printf("%s \n", templateName.toStdString().c_str());

        QString simCommand;
        simCommand = simCommand + "mevegs -i ";
        
        QString egsinpFile = templateName;
        for(std::size_t i = 0; i < allInputs.size(); i++){
            egsinpFile = egsinpFile + "_" + allInputs[i];
        }
        egsinpFile += ".egsinp &";

        simCommand += egsinpFile;

        printf("%s \n", simCommand.toStdString().c_str());

        // QProcess egsProcess;
        // egsProcess.start(simCommand);

        printf("start sim \n");

        // create new thread so that egs_view doesn't freeze while the simulation is running 
        QThread* thread = new QThread;
        sterigenicsWorker* worker = new sterigenicsWorker(simCommand, timeoutLimit);

        worker -> moveToThread(thread);
        connect(thread, &QThread::started, worker, &sterigenicsWorker::process);
        connect(worker, &sterigenicsWorker::simFinished, this, &sterigenicsTab::onSimulationFinished);
        connect(worker, &sterigenicsWorker::errorOccurred, this, &sterigenicsTab::onErrorOccurred);

        // connect(worker, &sterigenicsWorker::simFinished, thread, &QThread::quit);
        // connect(worker, &sterigenicsWorker::errorOccurred, thread, &QThread::quit);

        connect(worker, &sterigenicsWorker::simFinished, worker, &QThread::deleteLater);
        connect(worker, &sterigenicsWorker::errorOccurred, worker, &QThread::deleteLater);
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);
        // connect(thread, &QThread::finished, thread, &QThread::quit);

        thread -> start();
    }
}

void sterigenicsTab::openFileButtonClicked(){
    QString dirPath;
    dirPath = dirPath + getenv("EGS_HOME") + "mevegs";
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), filePath, tr("Mesh File (*.msh)"));

    QString fileName = filePath.section("/", -1, -1);

    ui -> meshFileLabel -> setText(fileName);
}

void sterigenicsTab::onSimulationFinished(const QString& output) {
    printf("%s \n", output.toStdString().c_str());
    printf("done sim\n");
}

void sterigenicsTab::onErrorOccurred(const QString& error) {
    printf("Process Error: %s\n", error.toStdString().c_str());
}